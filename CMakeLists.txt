#
# Example CMake file for the IHPCSS Software Engineering
# project, based on John Urbanic's laplace
# solver ported to use a C++ compiler.
#

cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(Software-engineering VERSION 0.3 LANGUAGES CXX)
set(PROJECT_VERSION_STRING "${PROJECT_VERSION}")

# ── Language standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE tooling (clangd, VSCode, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Output directories ─────────────────────────────────────────────────────────
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── Build type ─────────────────────────────────────────────────────────────────
# Do NOT use FORCE here — let the user (or CI) choose via -DCMAKE_BUILD_TYPE=…
# We only set a default if nothing was specified on the command line.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose type of build: Debug, MinSizeRel, Release, RelWithDebInfo")
endif()

# ── Options ────────────────────────────────────────────────────────────────────
option(BUILD_TESTS    "Enable unit test building" ON)
option(BUILD_COVERAGE "Instrument for gcov/lcov coverage (requires GCC or Clang and Debug build)" OFF)
option(BUILD_DOCS     "Enable Sphinx/Doxygen documentation targets" OFF)

option(MPI     "Enable MPI compiler support"    OFF)
option(OPENMP  "Enable OpenMP compiler support" OFF)
option(OPENACC "Enable OpenACC compiler support" OFF)

# Use GNUInstallDirs to set paths on multiarch systems.
include(GNUInstallDirs)

# ── Parallelisation support ────────────────────────────────────────────────────
# Modern CMake exposes MPI, OpenMP, and OpenACC as imported targets.
# Downstream targets should link against these targets directly with
#   target_link_libraries(mytarget PRIVATE MPI::MPI_CXX OpenMP::OpenMP_CXX)
# rather than appending global CMAKE_CXX_FLAGS strings.

if(MPI)
    find_package(MPI REQUIRED COMPONENTS CXX)
    if(MPI_CXX_FOUND)
        set(HAVE_MPI TRUE)
        message(STATUS "MPI found: ${MPI_CXX_COMPILER}")
    else()
        message(FATAL_ERROR "MPI support requested, but no CXX compiler support found.")
    endif()
endif()

if(OPENMP)
    find_package(OpenMP REQUIRED COMPONENTS CXX)
    if(OpenMP_CXX_FOUND)
        set(HAVE_OPENMP TRUE)
        message(STATUS "OpenMP found: version ${OpenMP_CXX_VERSION}")
    else()
        message(FATAL_ERROR "OpenMP support requested, but no CXX compiler support found.")
    endif()
endif()

if(OPENACC)
    find_package(OpenACC)
    if(OpenACC_CXX_FOUND)
        set(HAVE_OPENACC TRUE)
        set(OPENACC_VERSION ${OpenACC_CXX_VERSION})
        message(STATUS "OpenACC found: version ${OpenACC_CXX_VERSION}")
    else()
        message(FATAL_ERROR "OpenACC support requested, but no CXX compiler support found.")
    endif()
endif()

# ── Compiler warning flags ─────────────────────────────────────────────────────
include(CompilerFlags)

# ── Runtime sanitisers (optional) ──────────────────────────────────────────────
include(Sanitizers)

# ── Tests ──────────────────────────────────────────────────────────────────────
# enable_testing() must be called in the root CMakeLists.txt (not a subdirectory)
# for CTest to discover tests correctly.
if(BUILD_TESTS)
    enable_testing()
endif()

# ── Coverage ───────────────────────────────────────────────────────────────────
# Must be included after enable_testing() and before add_subdirectory(src) so
# the --coverage flags are applied to all compiled targets.
if(BUILD_COVERAGE)
    if(NOT BUILD_TESTS)
        message(FATAL_ERROR "BUILD_COVERAGE requires BUILD_TESTS=ON")
    endif()
    include(Coverage)
endif()

# ── Subdirectories ─────────────────────────────────────────────────────────────
add_subdirectory(src)

if(BUILD_DOCS)
    add_subdirectory(docs)
endif()
