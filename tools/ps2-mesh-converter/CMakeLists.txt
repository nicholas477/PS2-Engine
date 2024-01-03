project(PS2-Mesh-Converter CXX)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(PS2SDK_DIR "$ENV{PS2SDK}")


find_package(PkgConfig)
find_package(meshoptimizer REQUIRED CONFIG)
find_package(ps2-egg REQUIRED CONFIG)
pkg_check_modules(JSONCPP jsoncpp)

FILE(GLOB_RECURSE Source "src/**.cpp")

add_executable(egg-ps2-mesh-converter ${Source})

target_include_directories(egg-ps2-mesh-converter PRIVATE include "/usr/include/jsoncpp")
target_link_libraries(egg-ps2-mesh-converter PUBLIC meshoptimizer::meshoptimizer assimp ps2-egg jsoncpp)

set_property(TARGET egg-ps2-mesh-converter PROPERTY CXX_STANDARD 20)

install(TARGETS egg-ps2-mesh-converter DESTINATION "${PS2SDK_DIR}/bin")