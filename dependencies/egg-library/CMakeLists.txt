cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # Enables override of options from parent CMakeLists.txt
endif()

project(ps2-egg-library VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

FILE(GLOB Source "src/**.cpp" "src/**.cc")

add_library(ps2-egg ${Source})

target_include_directories(ps2-egg PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

set_property(TARGET ps2-egg PROPERTY CXX_STANDARD 20)

include(GNUInstallDirs)
install(TARGETS ps2-egg EXPORT ps2-egg-targets
    COMPONENT ps2-egg
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "include/egg" COMPONENT ps2-egg DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ps2-egg-targets COMPONENT ps2-egg DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ps2-egg)

include(CMakePackageConfigHelpers)

configure_package_config_file(config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ps2-eggConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ps2-egg NO_SET_AND_CHECK_MACRO)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ps2-eggConfigVersion.cmake COMPATIBILITY ExactVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ps2-eggConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ps2-eggConfigVersion.cmake
    COMPONENT ps2-egg
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ps2-egg)