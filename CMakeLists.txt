cmake_minimum_required(VERSION 3.0)

project(runds_connector VERSION 0.3.0)
add_definitions(-DVERSION="${PROJECT_VERSION}-dev")

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
    message(STATUS "Build type not specified: defaulting to release.")
endif()

include(GNUInstallDirs)

find_package(OwrxConnector 0.7 REQUIRED)

add_subdirectory(src)