# Software Engineering for the IHPCSS Laplace code

### Current status

[![CI](https://github.com/IHPCSS/software-engineering/actions/workflows/build-cmake.yml/badge.svg)](https://github.com/IHPCSS/software-engineering/actions/workflows/build-cmake.yml)
[![codecov](https://codecov.io/gh/IHPCSS/software-engineering/branch/main/graph/badge.svg)](https://codecov.io/gh/IHPCSS/software-engineering)
[![Documentation Status](https://readthedocs.org/projects/software-engineering/badge/?version=latest)](https://software-engineering.readthedocs.io/en/latest/)

# A Template Project for Software Engineering

As part of the IHPCSS summer school we show you a number of examples of good
practice in software engineering. However, just knowing about it does not help;
you need to apply it in your own work, and some of these tools are not entirely
trivial to configure.

To help you, I have used the Laplace programming example written by John
Urbanic and introduced reasonable software engineering standards, including:

* A GitHub repository
* CMake for build configuration (C++17, GCC and Clang tested)
* Continuous integration with GitHub Actions to automatically build and test
  every push: https://github.com/IHPCSS/software-engineering/actions
* High-level documentation with Sphinx, automatically published to ReadTheDocs:
  http://software-engineering.readthedocs.io/
* Source-code level documentation with Doxygen
* Unit tests with Google Test (run automatically by CI)

All these features have also been integrated in CMake so we check if the
necessary tools are available. For full documentation, consult the online
Sphinx documentation at ReadTheDocs: http://software-engineering.readthedocs.io/

This is of course massive overkill for the simple Laplace example. If you want
to see a real-world example, check out http://www.gromacs.org — but the
advantage of this project is that you can easily clone the repository and
replace the Laplace code with your own project. You are welcome to redistribute
the code in any way you want — no credit necessary.
