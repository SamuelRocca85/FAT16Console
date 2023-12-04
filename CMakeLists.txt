cmake_minimum_required(VERSION 3.27.8)

project(fatconsole)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(include)
