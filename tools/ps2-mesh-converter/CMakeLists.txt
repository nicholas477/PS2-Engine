project(PS2-Mesh-Converter CXX)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(PS2SDK_DIR "$ENV{PS2SDK}")


find_package(PkgConfig)
find_package(meshoptimizer REQUIRED CONFIG)

FILE(GLOB Source "src/**.cpp")

add_executable(egg-ps2-mesh-converter ${Source})

target_include_directories(egg-ps2-mesh-converter PRIVATE include)
target_include_directories(egg-ps2-mesh-converter PRIVATE "${PS2SDK_DIR}/ports/include")
target_link_libraries(egg-ps2-mesh-converter PUBLIC meshoptimizer::meshoptimizer assimp)

set_property(TARGET egg-ps2-mesh-converter PROPERTY CXX_STANDARD 20)

install(TARGETS egg-ps2-mesh-converter DESTINATION "${PS2SDK_DIR}/bin")