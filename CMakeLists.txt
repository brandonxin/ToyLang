cmake_minimum_required(VERSION 3.21)

project(toyc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(fmt REQUIRED)

add_subdirectory(src)
