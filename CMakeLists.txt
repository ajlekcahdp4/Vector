cmake_minimum_required(VERSION 3.18)

project(Vector LANGUAGES CXX)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source build is forbidden")
endif()

find_package(Threads REQUIRED)

find_package(GTest REQUIRED)
enable_testing()

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

include(GNUInstallDirs)

set(VECTOR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/include")
set(public_headers ${VECTOR_INCLUDE_DIR}/vector.hpp ${VECTOR_INCLUDE_DIR}/my_ranges.hpp)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${VECTOR_INCLUDE_DIR})
set_target_properties (${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")
install (TARGETS ${PROJECT_NAME}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
  )

add_subdirectory(unit_tests)
