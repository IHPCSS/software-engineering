.. _code_structure:

C++ Code Structure and Design
------------------------------

This page explains the design decisions behind the three core classes.
Understanding *why* the code is structured this way is as important as
understanding *what* it does — especially if you plan to extend it with
MPI, CUDA, or a different solver algorithm.

The ``Grid`` class
^^^^^^^^^^^^^^^^^^

``Grid`` is a uniform 2-D scalar field stored as a flat array of
``double`` values.  Its most important property is the **memory layout
contract**, which is part of the public interface and must never change:

.. code-block:: text

    element (i, j)  is located at  data()[j * nx + i]

Consecutive values in the x-direction (varying ``i``, fixed ``j``) are
therefore **contiguous in memory** — i.e. the layout is row-major with
rows running along the x-axis.

Why does the memory layout matter?
"""""""""""""""""""""""""""""""""""

Three reasons:

1. **Auto-vectorisation.**  The solver's innermost loop iterates over
   ``i`` (the x-direction).  Because those values are contiguous, the
   compiler can issue SIMD instructions (SSE, AVX, SVE, …) without any
   gather/scatter overhead.

2. **MPI halo exchange.**  When you partition the grid across MPI ranks
   by splitting rows, each ghost row is a contiguous block of ``nx``
   doubles.  You can pass ``grid.row(j)`` directly to
   ``MPI_Sendrecv`` without any packing step.

3. **CUDA kernels.**  Device memory is laid out identically.  A 2-D
   thread block can read a row slice with coalesced memory access by
   passing ``grid.row(j)`` to the kernel.

Raw-pointer access
""""""""""""""""""

``Grid`` exposes two raw-access functions alongside the index operator:

.. code-block:: cpp

    double*       data()         // pointer to the entire flat array
    double*       row(size_t j)  // pointer to the first element of row j

``row(j)`` is exactly ``data() + j * nx()``.  The solver uses ``row()``
to hoist pointer arithmetic outside the inner loop:

.. code-block:: cpp

    const double* above = grid.row(j + 1);
    const double* below = grid.row(j - 1);
    const double* mid   = grid.row(j);
    double*       out   = next.row(j);

    for (size_t i = 1; i < nx - 1; ++i) {
        out[i] = 0.25 * (mid[i-1] + mid[i+1] + below[i] + above[i]);
    }

The compiler now sees four independent stride-1 streams and can
auto-vectorise the inner loop without any index arithmetic in the hot path.

Bounds checking
"""""""""""""""

``operator()(i, j)`` and ``row(j)`` use ``assert()`` for bounds
checking.  In a **Debug** build (``NDEBUG`` not defined) out-of-range
accesses abort immediately with a clear message.  In a **Release** build
the assertions are compiled away and the functions reduce to a single
array index — zero overhead.

The ``BoundaryConditions`` hierarchy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The solver re-applies boundary conditions after every Jacobi sweep.
Rather than hard-coding a specific problem into the solver, we use a
simple abstract interface:

.. code-block:: cpp

    class BoundaryConditions {
    public:
        virtual ~BoundaryConditions() = default;
        virtual void apply(Grid& grid) const = 0;
    };

Any class that overrides ``apply()`` is a valid boundary condition.  The
solver never needs to know which concrete class it is talking to — it
just calls ``bc.apply(next)`` after every sweep.

This is the textbook *Strategy pattern*: the algorithm (Jacobi iteration)
is separated from the policy (what values go on the boundary).  To try a
new physical configuration — say, a periodic boundary, a Robin condition,
or a time-varying temperature — you derive a new class and pass it to
``solve()``.  The solver itself does not change.

Concrete implementations
"""""""""""""""""""""""""

``ConstantBC``
    Sets each of the four edges to a fixed temperature.  Bottom and top
    rows are written last and therefore own the four corner cells.

``CornerHeatBC``
    The original PSC/IHPCSS demo problem: left and top at 0 °C, right and
    bottom ramping linearly from 0 to 100 °C.

Corner ownership in ``ConstantBC``
"""""""""""""""""""""""""""""""""""

When two edges meet at a corner, one of them must "win".  ``ConstantBC``
writes left and right columns first, then bottom and top rows.  The
bottom and top passes overwrite the corner cells, so top and bottom own
all four corners.  ``CornerHeatBC`` uses the same convention: top row
and bottom row are written last.

This is a deliberate choice, not an accident.  Unit tests in
``src/laplace/tests/boundary_conditions.cpp`` verify the corner values
explicitly, so any future implementation that changes this convention
will be caught immediately.

The ``Solver``
^^^^^^^^^^^^^^

The free function ``solve()`` implements Jacobi iteration with
double-buffering:

.. code-block:: cpp

    SolverResult solve(Grid& grid, const BoundaryConditions& bc,
                       double tolerance = 0.01, size_t max_iter = 4000);

**Double-buffering** means we keep two grids: the current iterate
(``grid``) and the next iterate (``next``).  Each sweep reads from
``grid`` and writes to ``next``, then the two are swapped with
``std::swap`` — a zero-copy operation that only exchanges internal
pointers.  This is necessary for Jacobi (as opposed to Gauss-Seidel,
which updates in place) and makes the access pattern cache-friendly.

**Inline residual** — the maximum cell change is accumulated *during*
the sweep rather than in a second pass.  The data is therefore touched
exactly once per iteration.

**Convergence** is declared when the residual falls below ``tolerance``.
If ``max_iter`` sweeps complete without convergence, the solver returns
with ``converged = false``; the caller can inspect ``residual`` to decide
what to do.

Extension points
"""""""""""""""""

The comments in ``solver.h`` outline three natural ways to extend the
solver for HPC:

MPI (distributed memory)
    Partition the grid row-wise across MPI ranks.  Before each sweep,
    exchange the ghost rows (the top row of rank *k* and the bottom row
    of rank *k+1*) using ``MPI_Sendrecv``.  The ``grid.row()`` interface
    makes the send/receive buffers trivial to obtain.  The residual must
    be reduced across ranks with ``MPI_Allreduce``.

CUDA (GPU)
    Allocate device-side ``Grid`` objects with ``cudaMalloc`` and copy
    data from host with ``cudaMemcpy``.  Replace the inner loop with a
    2-D thread block where each thread computes one cell update.
    Double-buffering maps directly to two device buffers.

Gauss-Seidel / SOR
    Drop the ``next`` buffer and update ``grid`` in place.  For SOR,
    multiply each update by a relaxation factor :math:`\omega > 1`
    (optimal :math:`\omega` is typically between 1 and 2 for elliptic
    problems).  Convergence is typically several times faster than Jacobi.
