cmake_minimum_required(VERSION 3.4.1)

project(Fulgurous)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(GLFW_LOCATION "" CACHE PATH "Root of glfw install")

find_package(GLFW REQUIRED)

message(STATUS "CMAKE_SYSTEM_INFO_FILE = ${CMAKE_SYSTEM_INFO_FILE}")
message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")


add_definitions(-DGLFW_INCLUDE_NONE
				-D_CRT_SECURE_NO_WARNINGS
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

include_directories("inc" "filesystem" "glad/include" ${GLFW_INCLUDE_DIR} "stb" "nanovg/src" "sce_vectormath/include/vectormath/scalar/cpp")
add_executable(triangle "src/triangle.cpp" "src/utils.cpp" "src/shader.cpp" "glad/src/glad.c" "nanovg/src/nanovg.c" )
add_executable(ripple "src/triangle.cpp" "src/utils.cpp" "src/shader.cpp" "glad/src/glad.c" "nanovg/src/nanovg.c" )
target_link_libraries(triangle  ${GLFW_LIBRARIES})
target_link_libraries(ripple ${GLFW_LIBRARIES})
