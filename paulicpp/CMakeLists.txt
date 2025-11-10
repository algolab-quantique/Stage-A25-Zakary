cmake_minimum_required(VERSION 3.15)
project(pauliarray LANGUAGES C CXX)

# TODO: Test with other computers, notably Windows and Apple Silicon
# Current tests are on:
# OS: Linux Mint 22.2      || COMPILER: gcc 13.3.0            || CPU: AMD Ryzen 7 6800H
# OS: macOS Sequoia 15.7.1 || COMPILER: Homebrew clang 21.1.2 || CPU: i7 (quad core?)
# OS: macOS Sequoia 15.7.1 || COMPILER: Apple clang 17.0.0    || CPU: M1

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Option to require OpenMP. Set -DBUILD_WITH_OPENMP=ON to fail if OpenMP is not available.
# I dont know what happens when OpenMP is not found, with this option OFF.
# option(BUILD_WITH_OPENMP "Require OpenMP for builds (fail if not found)" ON)
option(USE_OPENMP "Enable OpenMP support" ON)


pybind11_add_module(_core
    pauliarray/_src/bindings/cz2m_bindings.cpp
    pauliarray/_src/bindings/bitops_bindings.cpp
    # pauliarray/_src/xxhash/xxhash.c 
)

# MUST-HAVES FLAGS
# O3 is the most aggressive optimization level that is safe. Ofast may break things, and is not a whole lot faster.
# march=native enables all instruction sets supported by the local machine. This causes non-portable binaries, but is faster in local builds.
#   Disabling the march flag may be useful when building pre-built binaries for distribution, but will kill any SIMD generation.
# fPIC is needed for shared libraries.

# OPTIONAL FLAGS
# flto=auto enables "Link Time Optimization", which supposedly allows for better optimization across the board (?). 
# However, i've not seen any gain nor loss in performance with it on or off.
# funroll-loops lets the compiler unroll loops. For now i wouldnt use it as i fear it may slow down things for giant NumPy arrays.
set(PAULI_COMPILE_OPTIONS
    -O3 
    -flto=auto
    -march=native
    -fPIC
    # -funroll-loops
)

# macOS: force libc++ and set rpath to avoid linking against an incompatible libstdc++
if(APPLE)
    foreach(target IN LISTS CMAKE_STATIC_LIBRARY_PREFIX)
        # giogiogighgfhghjhkgjkghk>.jhumgjyhtgy
    endforeach()
    # Add flags to compile with libc++ when using Apple clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)


    # Link against the system libc++ to make runtime symbols available
    find_library(SYSTEM_LIBCPP NAMES c++ libc++ PATHS /usr/lib /usr/local/lib)
    if(SYSTEM_LIBCPP)
        message(STATUS "Linking against system C++ library: ${SYSTEM_LIBCPP}")
        target_link_libraries(_core PRIVATE ${SYSTEM_LIBCPP})
    else()
        message(WARNING "System libc++ not found; runtime C++ symbols may be missing")
    endif()


    # Attempt to link libc++abi too
    find_library(SYSTEM_LIBCPPABI NAMES c++abi libc++abi PATHS /usr/lib /usr/local/lib)
    if(SYSTEM_LIBCPPABI)
        message(STATUS "Linking against system C++ ABI library: ${SYSTEM_LIBCPPABI}")
        target_link_libraries(_core PRIVATE ${SYSTEM_LIBCPPABI})
    else()
        message(STATUS "System libc++abi not found; continuing without explicit libc++abi link")
    endif()

    # Use Homebrew's LLVM libc++ if available
    find_library(HOMEBREW_LIBCPP NAMES libc++.1.0.dylib libc++.1.dylib libc++.dylib libc++ PATHS /usr/local/opt/llvm/lib/c++ /opt/homebrew/opt/llvm/lib/c++ /usr/local/opt/llvm/lib)
    if(HOMEBREW_LIBCPP)
        message(STATUS "Linking against Homebrew/LLVM libc++: ${HOMEBREW_LIBCPP}")
        # Link both modules against this explicit dylib so the symbol provider is available at runtime
        target_link_libraries(_core PRIVATE ${HOMEBREW_LIBCPP})
        # target_link_libraries(sparse_core PRIVATE ${HOMEBREW_LIBCPP})
        set(HOMEBREW_LIBCPP_DIR "${CMAKE_MATCH_LIBRARIES}/../c++" CACHE STRING "Calculated Homebrew libc++ dir" FORCE)

        set_target_properties(_core PROPERTIES INSTALL_RPATH "@loader_path;/usr/local/opt/llvm/lib/c++")
        # set_target_properties(sparse_core PROPERTIES INSTALL_RPATH "@loader_path;/usr/local/opt/llvm/lib/c++")
    else()
        message(STATUS "Homebrew libc++ not found in standard locations; will rely on system libcpython++")
    endif()
endif()


if(USE_OPENMP)
    find_package(OpenMP REQUIRED)

    if(NOT OpenMP_CXX_FOUND AND APPLE)
        find_library(LIBOMP_LIBRARY NAMES omp libomp PATHS /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib /usr/lib)
        find_path(LIBOMP_INCLUDE_DIR omp.h PATHS /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include /usr/include)

        if(LIBOMP_LIBRARY AND LIBOMP_INCLUDE_DIR)
            message(STATUS "Found libomp: ${LIBOMP_LIBRARY}, include: ${LIBOMP_INCLUDE_DIR}")
            # Provide flags as a CMake list so each token is passed separately to the compiler
            set(OpenMP_CXX_FLAGS -fopenmp -I${LIBOMP_INCLUDE_DIR} -DUSE_OPENMP)
            set(OpenMP_CXX_LIBRARIES ${LIBOMP_LIBRARY})
            set(OpenMP_CXX_FOUND TRUE)
        else()
            message(FATAL_ERROR "OpenMP not found. Install libomp (e.g. 'brew install libomp').")
        endif()
    endif()

    if(OpenMP_CXX_FOUND)
        # prefer modern importing if available
        if(TARGET OpenMP::OpenMP_CXX)
            target_link_libraries(_core PRIVATE OpenMP::OpenMP_CXX)
            # target_link_libraries(sparse_core PRIVATE OpenMP::OpenMP_CXX)
        else() # old maniere
            target_compile_options(_core PRIVATE ${OpenMP_CXX_FLAGS})
            target_link_libraries(_core PRIVATE ${OpenMP_CXX_LIBRARIES})
            # target_compile_options(sparse_core PRIVATE ${OpenMP_CXX_FLAGS})
            # target_link_libraries(sparse_core PRIVATE ${OpenMP_CXX_LIBRARIES})
        endif()

        target_compile_definitions(_core PRIVATE USE_OPENMP)
        # target_compile_definitions(sparse_core PRIVATE USE_OPENMP)

        if(DEFINED LIBOMP_INCLUDE_DIR)
            target_include_directories(_core PRIVATE ${LIBOMP_INCLUDE_DIR})
            # target_include_directories(sparse_core PRIVATE ${LIBOMP_INCLUDE_DIR})
        endif()
    endif()
endif()



set_target_properties(_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/_src/build
)

# set_target_properties(sparse_core PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/_src/build
# )

install(TARGETS _core DESTINATION pauliarray/_src/build)
# install(TARGETS sparse_core DESTINATION pauliarray/_src/build)





# Generate stub files
find_package(Python COMPONENTS Interpreter REQUIRED)



# TODO: Make this work with xxhash!
add_custom_command(TARGET _core POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}
          ${Python_EXECUTABLE} -m pybind11_stubgen
          pauliarray._src.build._core
          -o ${CMAKE_SOURCE_DIR}
          )

# add_custom_command(TARGET sparse_core POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}
#           ${Python_EXECUTABLE} -m pybind11_stubgen
#           pauliarray._src.build.sparse_core
#           -o ${CMAKE_SOURCE_DIR}
#           )



# ================== DOES NOT WORK! USE PYBIND11_STUBGEN INSTEAD ==================

# add_custom_command(TARGET _core POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}
#                     ${Python_EXECUTABLE} -m mypy.stubgen --module
#                     pauliarray._src.build._core
#                     -o ${CMAKE_SOURCE_DIR}
# )

# add_custom_command(TARGET sparse_core POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_SOURCE_DIR}
#                     ${Python_EXECUTABLE} -m mypy.stubgen --module
#                     pauliarray._src.build.sparse_core
#                     -o ${CMAKE_SOURCE_DIR}
# )