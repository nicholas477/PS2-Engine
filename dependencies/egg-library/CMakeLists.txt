project(ps2-egg-library CXX)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

FILE(GLOB Source "src/**.cpp" "src/**.cc")

add_library(ps2-egg ${Source})

target_include_directories(ps2-egg PUBLIC include)

set_property(TARGET ps2-egg PROPERTY CXX_STANDARD 20)