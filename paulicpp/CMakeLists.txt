cmake_minimum_required(VERSION 3.15)
project(pauliarray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)


pybind11_add_module(pauliarray
    pauliarray/src/pauliarray.cpp
    pauliarray/src/voidops.cpp
)

install(TARGETS pauliarray DESTINATION .)