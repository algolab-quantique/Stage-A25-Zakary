cmake_minimum_required(VERSION 3.15)
project(pauliarray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) #TODO: Change enventually to 20. See voidops.hpp to see why
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
find_package(OpenMP REQUIRED)                

pybind11_add_module(pauliarray
    pauliarray/src/bindings/paulicpp_bindings.cpp
)
pybind11_add_module(voidops
    pauliarray/src/bindings/voidops_bindings.cpp
)

set(PAULI_COMPILE_OPTIONS
    -O3
    -flto=auto
    -march=native
    -fPIC
)

find_package(OpenMP)

if(NOT OpenMP_CXX_FOUND AND APPLE)
    find_library(LIBOMP_LIBRARY NAMES omp libomp PATHS /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib /usr/lib)
    find_path(LIBOMP_INCLUDE_DIR omp.h PATHS /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include /usr/include)

    if(LIBOMP_LIBRARY AND LIBOMP_INCLUDE_DIR)
        message(STATUS "Found libomp: ${LIBOMP_LIBRARY}, include: ${LIBOMP_INCLUDE_DIR}")

        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${LIBOMP_INCLUDE_DIR}")
        set(OpenMP_CXX_LIBRARIES ${LIBOMP_LIBRARY})
        set(OpenMP_CXX_FOUND TRUE)
    else()
        message(WARNING "OpenMP not found!")
    endif()
endif()

if(OpenMP_CXX_FOUND)
    target_compile_options(pauliarray PRIVATE ${OpenMP_CXX_FLAGS})
    target_compile_options(voidops PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(pauliarray PRIVATE ${OpenMP_CXX_LIBRARIES})
    target_link_libraries(voidops PRIVATE ${OpenMP_CXX_LIBRARIES})
endif()


set_target_properties(voidops PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)
set_target_properties(pauliarray PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)

install(TARGETS pauliarray DESTINATION .)
install(TARGETS voidops DESTINATION .)