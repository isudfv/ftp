cmake_minimum_required(VERSION 3.16)
project(ftp)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(server ${PROJECT_SOURCE_DIR}/src/server.cpp)
add_executable(client ${PROJECT_SOURCE_DIR}/src/client.cpp)
