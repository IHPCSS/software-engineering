.. _software_engineering:

Software Engineering
--------------------

Git & GitHub for source code tracking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The first practice you must adopt is revision control. As you can see from
https://github.com/IHPCSS/software-engineering, development of even a small
project goes through many iterations, and without revision control you would
repeatedly face the problem of trying to fix errors you introduced yourself.
With revision control you can simply step back to a known-good version instead.

For a large and very active project like GROMACS, we run our own server where
every single commit is tracked, and all developers continuously rebase their
changes onto the current state of the main branch. Another common approach
(used e.g. on GitHub) is the *pull request* model: a developer sends a PR,
and the maintainer reviews and merges it. Either model works.

If you are just starting out, we strongly recommend creating an account at
https://github.com and putting your repositories there. GitHub has plenty of
documentation on how to set up an empty repository or import existing code.


Issue Tracking
^^^^^^^^^^^^^^

Even for a small project with a single developer, it is valuable to have a
list where you can note bugs, features, and other considerations. GitHub
includes an integrated issue tracker — click the "Issues" tab and create a
new one. Each issue gets a unique number, and you can close it automatically
from a commit message::

    Fixes #14

A file ``README.md`` in the top-level directory is shown as the default page
for anyone visiting the GitHub repository. It is a good place for a brief
description and status badges (see below).


CMake for Build Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We use CMake (https://cmake.org) for build configuration — it works on
Linux, macOS, and Windows, and has excellent support for MPI, OpenMP,
CUDA, and many other HPC tools.

The canonical out-of-source build workflow is::

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel

We require CMake 3.20 or later. Modern CMake uses *imported targets* for
dependencies instead of raw compiler flags. For example::

    target_link_libraries(laplace_lib PUBLIC MPI::MPI_CXX)

This automatically propagates the right include paths, compile flags, and
linker flags to anything that links against ``laplace_lib`` — you never
touch ``CMAKE_CXX_FLAGS`` directly.

Useful CMake variables
""""""""""""""""""""""

``CMAKE_BUILD_TYPE``
    ``Release`` (default, with optimisation), ``Debug`` (with assertions
    and debug symbols), or ``RelWithDebInfo``.

``BUILD_TESTS``
    ``ON`` (default) to compile and register the GoogleTest unit tests.

``BUILD_COVERAGE``
    ``ON`` to instrument the build for gcov/lcov coverage reporting
    (requires GCC or Clang and a Debug build).

``BUILD_DOCS``
    ``ON`` to enable the Sphinx and Doxygen documentation targets.

``MPI`` / ``OPENMP`` / ``OPENACC``
    ``ON`` to search for and link against the respective parallelisation
    library.

To list all available options interactively, run ``ccmake <path-to-source>``.

Multiple build directories
""""""""""""""""""""""""""

Rather than reconfiguring the same directory, keep separate build directories
for different configurations::

    cmake -B build-debug   -DCMAKE_BUILD_TYPE=Debug   -DBUILD_TESTS=ON
    cmake -B build-release -DCMAKE_BUILD_TYPE=Release
    cmake -B build-mpi     -DCMAKE_BUILD_TYPE=Release -DMPI=ON

A change to one source file only requires a recompile in the affected
directory.


GitHub Actions for Continuous Integration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Every push to GitHub automatically triggers a CI pipeline defined in
``.github/workflows/build-cmake.yml``. The pipeline runs a *matrix* of
configurations: GCC and Clang, each in Debug and Release mode, on
Ubuntu 24.04. This catches problems that only appear with a specific
compiler or optimisation level before they reach the main branch.

The matrix is defined in the workflow file as:

.. code-block:: yaml

    strategy:
      fail-fast: false
      matrix:
        compiler:   [gcc, clang]
        build_type: [Debug, Release]

``fail-fast: false`` ensures that all four combinations run even if one
fails — you see the full picture rather than stopping at the first error.

A separate ``coverage`` job runs after the matrix succeeds. It builds with
``-DBUILD_COVERAGE=ON`` and uploads the resulting coverage report to Codecov
(see :ref:`coverage` below).

The CI status is shown as a badge in the ``README.md`` file so that anyone
visiting the repository immediately sees whether the current code passes all
checks.

To add CI to your own project, copy ``.github/workflows/build-cmake.yml``
into your repository, adjust the compiler and build-type matrix to match
your requirements, and push. GitHub Actions is free for public repositories.


.. _coverage:

Code Coverage
^^^^^^^^^^^^^

*Code coverage* measures the fraction of your source lines that are
actually executed by the test suite. 100% line coverage means every line
of production code was reached by at least one test; it does not guarantee
correctness, but it does guarantee that untested code paths cannot silently
break.

This project uses **gcov** (the GCC coverage tool) and **lcov** (a
front-end that collects gcov data and renders an HTML report). The CMake
build integrates both through the ``coverage`` target::

    cmake -B build -DBUILD_TESTS=ON -DBUILD_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --target coverage
    open build/coverage-report/index.html

``cmake/Coverage.cmake`` auto-detects the gcov version that matches the
compiler (e.g. ``gcov-14`` when using ``g++-14``) and passes it to lcov
via ``--gcov-tool``, avoiding the version-mismatch error that occurs when
the system ``gcov`` is a different version from the compiler.

The ``coverage`` job in CI uploads the ``coverage.info`` file to
**Codecov** (https://codecov.io), which hosts an annotated HTML report
and provides the coverage badge shown in ``README.md``. Codecov highlights
which lines are covered (green) and which are not (red), making it easy to
identify gaps in the test suite.

The ``codecov.yml`` configuration file (in ``.github/``) excludes the
vendored GoogleTest sources from the report so the badge reflects coverage
of *our* code only.


General Documentation with Sphinx
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The documentation you are reading right now is generated by Sphinx, which
reads plain-text reStructuredText (``.rst``) files from the ``docs``
subdirectory. The raw files are readable in any editor, and Sphinx can
produce HTML, PDF (via LaTeX), and other formats.

If Sphinx is installed locally::

    cmake -B build -DBUILD_DOCS=ON
    cmake --build build --target sphinx-html
    open build/docs/html/index.html

You do not need a local Sphinx installation: ReadTheDocs (https://readthedocs.org)
integrates with GitHub and rebuilds the documentation automatically on every
push. Connect ReadTheDocs to your repository once, and anyone can read the
current documentation at a link like https://software-engineering.readthedocs.io/.

The badge in ``README.md`` shows the status of the last ReadTheDocs build.


Code Documentation with Doxygen
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sphinx provides high-level narrative documentation. Doxygen parses the C++
source code and generates reference documentation — class hierarchies,
function signatures, parameter descriptions, and cross-references — directly
from the ``///`` and ``/** */`` comments in the header files.

If Doxygen is installed::

    cmake -B build -DBUILD_DOCS=ON
    cmake --build build --target doxygen
    open build/docs/doxygen/html/index.html

All three library headers (``grid.h``, ``boundary_conditions.h``,
``solver.h``) are fully documented with Doxygen comments, including the
memory layout contract, class invariants, and extension points for MPI and
CUDA.


Unit Tests with GoogleTest
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Continuous integration tells you the code compiles. Unit tests tell you it
also *works correctly*.

The key idea is to design your code as small, independent modules with a
clear interface, and test each module in isolation. If the tests are small
and cover every code path, the CI system can tell you *exactly* which
20–30 lines introduced a regression — before you have even opened the
source code.

This project uses **GoogleTest**, which is vendored under
``src/external/googletest/`` so you do not need to install it separately.
Tests are organised in ``src/laplace/tests/``, one file per library source:

``grid.cpp``
    Tests for the ``Grid`` class: construction invariants, memory layout
    contract, ``write()`` success and failure paths.

``boundary_conditions.cpp``
    Tests for ``ConstantBC`` and ``CornerHeatBC``: edge values, corner
    ownership, interior cells left untouched.

``solver.cpp``
    Tests for ``solve()``: uniform boundary conditions (converges in one
    sweep), single-interior-cell exact solution, convergence within
    ``max_iter``, ``SolverResult`` field consistency.

To build and run the tests::

    cmake -B build -DBUILD_TESTS=ON
    cmake --build build --target tests
    ctest --test-dir build --output-on-failure

Or to build and run in one step::

    cmake --build build --target check

The current test suite achieves **100% line and branch coverage** of all
three library source files, verified by gcov/lcov (see :ref:`coverage`).


Source Code Organisation
^^^^^^^^^^^^^^^^^^^^^^^^

::

    software-engineering/
    ├── .github/
    │   ├── codecov.yml          # Codecov reporting configuration
    │   └── workflows/
    │       └── build-cmake.yml  # GitHub Actions CI pipeline
    ├── cmake/
    │   ├── CompilerFlags.cmake  # Warning flags for GCC, Clang, Intel
    │   ├── Coverage.cmake       # gcov/lcov coverage support
    │   └── TestMacros.cmake     # add_unit_test() helper macro
    ├── docs/                    # Sphinx source (this documentation)
    │   └── doxygen/             # Doxygen configuration
    ├── src/
    │   ├── external/
    │   │   └── googletest/      # Vendored GoogleTest (not our code)
    │   ├── laplace/             # Library: Grid, BoundaryConditions, Solver
    │   │   ├── grid.h / .cpp
    │   │   ├── boundary_conditions.h / .cpp
    │   │   ├── solver.h / .cpp
    │   │   └── tests/           # Unit tests (one file per source)
    │   │       ├── grid.cpp
    │   │       ├── boundary_conditions.cpp
    │   │       └── solver.cpp
    │   └── program/             # Executable (main.cpp)
    └── CMakeLists.txt

The library (``laplace_lib``) is compiled as a static library so that the
test executables can link against it independently of the ``laplace``
executable. This is a deliberate software engineering choice: it means the
solver logic is testable in isolation without running the full program.

Header and implementation files are co-located in ``src/laplace/`` rather
than separated into ``include/`` and ``src/`` directories. The ``include/``
separation is the right choice for *installable* libraries (where users
install headers separately from binaries), but for a self-contained
teaching project it adds indirection without benefit.
