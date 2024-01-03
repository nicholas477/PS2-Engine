cmake_minimum_required(VERSION 3.10)
project(PS2-Manifest-Generator CXX)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(PS2SDK_DIR "$ENV{PS2SDK}")


find_package(PkgConfig)
find_package(ps2-egg REQUIRED CONFIG)

FILE(GLOB Source "src/**.cpp")

add_executable(egg-ps2-manifest-generator ${Source})

target_include_directories(egg-ps2-manifest-generator PRIVATE include)
target_link_libraries(egg-ps2-manifest-generator PUBLIC ps2-egg)

set_property(TARGET egg-ps2-manifest-generator PROPERTY CXX_STANDARD 20)

install(TARGETS egg-ps2-manifest-generator DESTINATION "${PS2SDK_DIR}/bin")