cmake_minimum_required(VERSION 3.15)
project(
    z2r_accel
    LANGUAGES C
              CXX)

include_directories(${CMAKE_SOURCE_DIR}/z2r_accel/_core/include)

# TODO: Test with other computers, notably Windows and Apple Silicon Current tests are on: OS: Linux
# Mint 22.2      || COMPILER: gcc 13.3.0            || CPU: AMD Ryzen 7 6800H OS: macOS Sequoia
# 15.7.1 || COMPILER: Homebrew clang 21.1.2 || CPU: i7 (quad core?) OS: macOS Sequoia 15.7.1 ||
# COMPILER: Apple clang 17.0.0    || CPU: M1

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(
    pybind11
    CONFIG
    REQUIRED)

# Option to require OpenMP. Set -DBUILD_WITH_OPENMP=ON to fail if OpenMP is not available. I dont
# know what happens when OpenMP is not found, with this option OFF. option(BUILD_WITH_OPENMP
# "Require OpenMP for builds (fail if not found)" ON)
option(
    USE_OPENMP
    "Enable OpenMP support"
    ON)

# Pybind11 modules will be configured via the helper function below to avoid duplicating the same
# settings for each module.

# MUST-HAVES FLAGS 
# - O3 is the most aggressive optimization level that is safe. Ofast may break
# things, and is not a whole lot faster. 
# - march=native enables all instruction sets supported by the
# local machine. This causes non-portable binaries, but is faster in local builds. Disabling the
# march flag may be useful when building pre-built binaries for distribution, but will probably kill any SIMD
# generation. 
# - fPIC is needed for shared libraries.

# OPTIONAL FLAGS:
# - flto=auto enables "Link Time Optimization", which supposedly allows for better
# optimization across the board (?). However, i've not seen any gain nor loss in performance with it
# on or off. 
# - funroll-loops lets the compiler unroll loops. For now i wouldnt use it as i fear it may
# slow down things for giant NumPy arrays.
# - Wall enables all warnings. Useful for development. Not for production
#
# For more options, see: https://clang.llvm.org/docs/ClangCommandLineReference.html
# and https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
# The majority of flags supported by clang are also supported on GCC, and vice-versa.
set(PAULI_COMPILE_OPTIONS
    -O3
    -flto=auto
    -march=native
    -fPIC
    # -funroll-loops
    # -Wall
)

# macOS: force libc++ and set rpath to avoid linking against an incompatible libstdc++
if(APPLE)
    foreach(
        target
        IN
        LISTS CMAKE_STATIC_LIBRARY_PREFIX)
        # giogiogighgfhghjhkgjkghk>.jhumgjyhtgy
    endforeach()
    # Add flags to compile with libc++ when using Apple clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

    # Link against the system libc++ to make runtime symbols available
    find_library(
        SYSTEM_LIBCPP
        NAMES c++
              libc++
        PATHS /usr/lib
              /usr/local/lib)
    if(SYSTEM_LIBCPP)
        message(STATUS "Linking against system C++ library: ${SYSTEM_LIBCPP}")
        # Per-target linking of ${SYSTEM_LIBCPP} will be handled when each pybind11 module target is
        # created by configure_pybind_module().
    else()
        message(WARNING "System libc++ not found; runtime C++ symbols may be missing")
    endif()

    # Attempt to link libc++abi too
    find_library(
        SYSTEM_LIBCPPABI
        NAMES c++abi
              libc++abi
        PATHS /usr/lib
              /usr/local/lib)
    if(SYSTEM_LIBCPPABI)
        message(STATUS "Linking against system C++ ABI library: ${SYSTEM_LIBCPPABI}")
        # Per-target linking of ${SYSTEM_LIBCPPABI} will be handled later.
    else()
        message(STATUS "System libc++abi not found; continuing without explicit libc++abi link")
    endif()

    # Use Homebrew's LLVM libc++ if available
    find_library(
        HOMEBREW_LIBCPP
        NAMES libc++.1.0.dylib
              libc++.1.dylib
              libc++.dylib
              libc++
        PATHS /usr/local/opt/llvm/lib/c++
              /opt/homebrew/opt/llvm/lib/c++
              /usr/local/opt/llvm/lib)
    if(HOMEBREW_LIBCPP)
        message(STATUS "Linking against Homebrew/LLVM libc++: ${HOMEBREW_LIBCPP}")
        # Link both modules against this explicit dylib so the symbol provider is available at
        # runtime Per-target linking and rpath properties will be applied later in
        # configure_pybind_module() so we don't reference targets before they exist.
        set(HOMEBREW_LIBCPP_DIR
            "${CMAKE_MATCH_LIBRARIES}/../c++"
            CACHE STRING
                  "Calculated Homebrew libc++ dir"
                  FORCE)
    else()
        message(
            STATUS
                "Homebrew libc++ not found in standard locations; will rely on system libcpython++")
    endif()
endif()

if(USE_OPENMP)
    find_package(
        OpenMP
        REQUIRED)

    if(NOT
       OpenMP_CXX_FOUND
       AND APPLE)
        find_library(
            LIBOMP_LIBRARY
            NAMES omp
                  libomp
            PATHS /opt/homebrew/opt/libomp/lib
                  /usr/local/opt/libomp/lib
                  /usr/lib)
        find_path(
            LIBOMP_INCLUDE_DIR
            omp.h
            PATHS /opt/homebrew/opt/libomp/include
                  /usr/local/opt/libomp/include
                  /usr/include)

        if(LIBOMP_LIBRARY AND LIBOMP_INCLUDE_DIR)
            message(STATUS "Found libomp: ${LIBOMP_LIBRARY}, include: ${LIBOMP_INCLUDE_DIR}")
            # Provide flags as a CMake list so each token is passed separately to the compiler
            set(OpenMP_CXX_FLAGS
                -fopenmp
                -I${LIBOMP_INCLUDE_DIR}
                -DUSE_OPENMP)
            set(OpenMP_CXX_LIBRARIES ${LIBOMP_LIBRARY})
            set(OpenMP_CXX_FOUND TRUE)
        else()
            message(FATAL_ERROR "OpenMP not found. Install libomp (e.g. 'brew install libomp').")
        endif()
    endif()

    if(OpenMP_CXX_FOUND)
        # prefer modern importing if available
        if(TARGET
           OpenMP::OpenMP_CXX)
            # Per-target OpenMP linking will be handled later per-target.
        else() # old maniere
            # Per-target OpenMP compile/link settings will be applied when creating each module
            # target.
        endif()

        # The USE_OPENMP compile definition and any include dirs will be added to each target inside
        # configure_pybind_module().
    endif()
endif()

find_package(Python COMPONENTS Interpreter REQUIRED)

# Function name is 'configure_pybind_module'. TARGET_NAME: Name of the pybind11 module target to
# create SRC: Path to the binding file of the module ({module}_bindings.cpp)
function(
    configure_pybind_module
    TARGET_NAME
    SRC)
    pybind11_add_module(
        ${TARGET_NAME}
        ${SRC}
        ${ARGN})

    # common compile options
    target_compile_options(${TARGET_NAME} PRIVATE ${PAULI_COMPILE_OPTIONS})

    # OpenMP settings if available
    if(USE_OPENMP AND OpenMP_CXX_FOUND)
        if(TARGET
           OpenMP::OpenMP_CXX)
            target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)
        else()
            target_compile_options(${TARGET_NAME} PRIVATE ${OpenMP_CXX_FLAGS})
            target_link_libraries(${TARGET_NAME} PRIVATE ${OpenMP_CXX_LIBRARIES})
        endif()
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENMP)
        if(DEFINED
           LIBOMP_INCLUDE_DIR)
            target_include_directories(${TARGET_NAME} PRIVATE ${LIBOMP_INCLUDE_DIR})
        endif()
    endif()

    # macOS-specific C++ library linking if discovered earlier
    if(APPLE)
        if(SYSTEM_LIBCPP)
            target_link_libraries(${TARGET_NAME} PRIVATE ${SYSTEM_LIBCPP})
        endif()
        if(SYSTEM_LIBCPPABI)
            target_link_libraries(${TARGET_NAME} PRIVATE ${SYSTEM_LIBCPPABI})
        endif()
        if(HOMEBREW_LIBCPP)
            target_link_libraries(${TARGET_NAME} PRIVATE ${HOMEBREW_LIBCPP})
            set_target_properties(
                ${TARGET_NAME}
                PROPERTIES INSTALL_RPATH
                           "@loader_path;/usr/local/opt/llvm/lib++")
        endif()
    endif()

    # output location and install
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                   ${CMAKE_SOURCE_DIR}/z2r_accel/_core/build)
    install(TARGETS ${TARGET_NAME} DESTINATION z2r_accel/_core/build)

    # post-build: generate Python stubs
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR} ${Python_EXECUTABLE} -m
                pybind11_stubgen z2r_accel._core.build.${TARGET_NAME} -o ${CMAKE_SOURCE_DIR})
endfunction()

# add any ne wmodules here, with the name of the .so and the binding file
configure_pybind_module(
    _cz2m
    z2r_accel/_core/bindings/cz2m_bindings.cpp
    z2r_accel/_core/src/cz2m.cpp
    z2r_accel/_core/src/bitops.cpp)
configure_pybind_module(
    _bitops
    z2r_accel/_core/bindings/bitops_bindings.cpp
    z2r_accel/_core/src/bitops.cpp)
