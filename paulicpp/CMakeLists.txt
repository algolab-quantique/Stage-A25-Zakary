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

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND PAULI_COMPILE_OPTIONS -fopenmp=libomp)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND PAULI_COMPILE_OPTIONS -fopenmp)
endif()

target_compile_options(pauliarray PRIVATE ${PAULI_COMPILE_OPTIONS})
target_compile_options(voidops PRIVATE ${PAULI_COMPILE_OPTIONS})

target_link_libraries(pauliarray PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(voidops PRIVATE OpenMP::OpenMP_CXX)

# target_link_options(pauliarray PRIVATE -flto=auto)
# target_link_options(voidops PRIVATE -flto=auto)

set_target_properties(voidops PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)
set_target_properties(pauliarray PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/pauliarray/src/build
)

install(TARGETS pauliarray DESTINATION .)
install(TARGETS voidops DESTINATION .)