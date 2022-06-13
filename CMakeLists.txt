cmake_minimum_required(VERSION 3.4.1)

project(fulgurous)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")


message(STATUS "CMAKE_SYSTEM_INFO_FILE = ${CMAKE_SYSTEM_INFO_FILE}")
message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")

file(GLOB PROJECT_SOURCES src/*.cpp)
file(GLOB PROJECT_HEADERS src/*.h)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})

add_library(fulgurous STATIC ${PROJECT_SOURCES} ${PROJECT_HEADERS})

add_executable(triangle "examples/triangle.cpp" "glad/src/glad.c" "nanovg/src/nanovg.c" )
add_executable(ripple "examples/ripple.cpp" "glad/src/glad.c" "nanovg/src/nanovg.c" )
add_definitions(-D_CRT_SECURE_NO_WARNINGS
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_subdirectory(glfw)
include_directories("${PROJECT_SOURCE_DIR}/inc" "filesystem" "glad/include" "glfw/include" "stb" "nanovg/src" "sce_vectormath/include/vectormath/scalar/cpp")
target_link_libraries(triangle PRIVATE glfw fulgurous)
target_link_libraries(ripple PRIVATE glfw fulgurous)
