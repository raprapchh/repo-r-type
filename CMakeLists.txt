cmake_minimum_required(VERSION 3.20)

project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(r-type_client src/client/main.cpp)
add_executable(r-type_server src/server/main.cpp)
