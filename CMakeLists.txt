cmake_minimum_required(VERSION 3.0)

project(runds_connector)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
    message(STATUS "Build type not specified: defaulting to release.")
endif()

include(FindPkgConfig)
include(GNUInstallDirs)

pkg_check_modules(OWRX_CONNECTOR REQUIRED owrx-connector)

add_subdirectory(src)