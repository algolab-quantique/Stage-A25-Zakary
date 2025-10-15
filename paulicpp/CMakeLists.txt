cmake_minimum_required(VERSION 3.15)
project(pauliarray LANGUAGES CXX)

# TODO: Test with other computers, notably Windows and Apple Silicon
# Current tests are on:
# OS: Linux Mint 22.2      || COMPILER: gcc 13.3.0            || CPU: AMD Ryzen 7 6800H
# OS: macOS Sequoia 15.7.1 || COMPILER: Homebrew clang 21.1.2 || CPU: i7 (quad core?)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Option to require OpenMP. Set -DBUILD_WITH_OPENMP=ON to fail if OpenMP is not available.
# I dont know what happens when OpenMP is not found, with this option OFF.
option(BUILD_WITH_OPENMP "Require OpenMP for builds (fail if not found)" ON)

find_package(OpenMP)

pybind11_add_module(paulicpp
    pauliarray/src/bindings/paulicpp_bindings.cpp
    pauliarray/src/bindings/voidops_bindings.cpp
)
pybind11_add_module(sparsepaulicpp
    pauliarray/src/bindings/sparsepaulicpp_bindings.cpp
)

# MUST-HAVES FLAGS
# O3 is the most aggressive optimization level that is not likely to break things. Ofast may break things, and is not a whole lot faster.
# march=native enables all instruction sets supported by the local machine. This causes non-portable binaries, but is faster in local builds.
# fPIC is needed for shared libraries.

# OPTIONAL FLAGS
# flto=auto enables "Link Time Optimization", which supposedly allows for better optimization across the board. However, i've not seen any gain nor loss in performance with it on or off.
# finroll-loops lets the compiler unroll loops. For now i wouldnt use it as i fear it may slow down things for giant NumPy arrays.
set(PAULI_COMPILE_OPTIONS
    -O3 
    -flto=auto
    -march=native
    -fPIC
    # -funroll-loops
)

# macOS: force libc++ and set rpath to avoid linking against an incompatible libstdc++
if(APPLE)
    message(STATUS "Applying macOS specific compile/link flags: use libc++ and set rpath")
    foreach(target IN LISTS CMAKE_STATIC_LIBRARY_PREFIX)
        # 
    endforeach()
    # Add flags to compile with libc++ when using Apple clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

if(APPLE)
    # Ensure we explicitly link against the system libc++ to make runtime symbols available
    find_library(SYSTEM_LIBCPP NAMES c++ libc++ PATHS /usr/lib /usr/local/lib)
    if(SYSTEM_LIBCPP)
        message(STATUS "Linking against system C++ library: ${SYSTEM_LIBCPP}")
        target_link_libraries(paulicpp PRIVATE ${SYSTEM_LIBCPP})
    else()
        message(WARNING "System libc++ not found; runtime C++ symbols may be missing")
    endif()
endif()

find_package(OpenMP)

if(NOT OpenMP_CXX_FOUND AND APPLE)
    find_library(LIBOMP_LIBRARY NAMES omp libomp PATHS /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib /usr/lib)
    find_path(LIBOMP_INCLUDE_DIR omp.h PATHS /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include /usr/include)

    if(LIBOMP_LIBRARY AND LIBOMP_INCLUDE_DIR)
        message(STATUS "Found libomp: ${LIBOMP_LIBRARY}, include: ${LIBOMP_INCLUDE_DIR}")
        # Provide flags as a CMake list so each token is passed separately to the compiler
        set(OpenMP_CXX_FLAGS -Xpreprocessor -fopenmp -I${LIBOMP_INCLUDE_DIR})
        set(OpenMP_CXX_LIBRARIES ${LIBOMP_LIBRARY})
        set(OpenMP_CXX_FOUND TRUE)
    else()
        if(BUILD_WITH_OPENMP)
            message(FATAL_ERROR "OpenMP not found. Install libomp (e.g. 'brew install libomp').")
        else()
            message(WARNING "OpenMP not found! Trying to build without OpenMP parallelization.")
        endif()
    endif()
endif()

if(OpenMP_CXX_FOUND)
    target_compile_options(paulicpp PRIVATE ${OpenMP_CXX_FLAGS})
    # Ensure the OpenMP include dir is added so the -I flag points to something
    if(DEFINED LIBOMP_INCLUDE_DIR)
        target_include_directories(paulicpp PRIVATE ${LIBOMP_INCLUDE_DIR})
    endif()
    target_link_libraries(paulicpp PRIVATE ${OpenMP_CXX_LIBRARIES})
else()
    if(BUILD_WITH_OPENMP)
        message(FATAL_ERROR "OpenMP was requested (BUILD_WITH_OPENMP=ON) but could not be enabled.")
    endif()
endif()


set_target_properties(paulicpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)

set_target_properties(sparsepaulicpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)

install(TARGETS paulicpp DESTINATION pauliarray/src/build)

install(TARGETS sparsepaulicpp DESTINATION pauliarray/src/build)
