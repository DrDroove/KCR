cmake_minimum_required(VERSION 3.14)
project(KCR)

set(PYBIND11_FINDPYTHON ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development)
execute_process(
    COMMAND python3 -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)



find_package(pybind11 REQUIRED)

pybind11_add_module(solver solver.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(solver PRIVATE -O3)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(solver PRIVATE /O2)
endif()