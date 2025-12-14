cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
add_definitions(-DASIO_STANDALONE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

#---------------------------------------------------------------------------
# Dependencies (via vcpkg)
#---------------------------------------------------------------------------
# ASIO is header-only
find_path(ASIO_INCLUDE_DIR asio.hpp)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "ASIO not found. Install via: vcpkg install asio")
endif()
include_directories(${ASIO_INCLUDE_DIR})

# EnTT
find_package(EnTT CONFIG REQUIRED)

# SFML - from system (install with: sudo dnf install SFML-devel)
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
set(SFML_LIBS sfml-graphics sfml-window sfml-system sfml-audio)

# Catch2 is optional (for tests only)
find_package(Catch2 3 CONFIG QUIET)

#---------------------------------------------------------------------------
# ECS Library
#---------------------------------------------------------------------------
file(GLOB_RECURSE ECS_SRC ${CMAKE_SOURCE_DIR}/ecs/src/*.cpp)

add_library(rtype_ecs ${ECS_SRC})
target_include_directories(rtype_ecs
    PUBLIC
        ${CMAKE_SOURCE_DIR}/ecs/include
        ${CMAKE_SOURCE_DIR}/shared
)
target_link_libraries(rtype_ecs PUBLIC EnTT::EnTT ${SFML_LIBS})

#---------------------------------------------------------------------------
# Shared Library (headers only)
#---------------------------------------------------------------------------
add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE ${CMAKE_SOURCE_DIR}/shared)

#---------------------------------------------------------------------------
# Client Executable
#---------------------------------------------------------------------------
file(GLOB_RECURSE CLIENT_SRC ${CMAKE_SOURCE_DIR}/client/src/*.cpp)

add_executable(r-type_client ${CLIENT_SRC})
target_include_directories(r-type_client PRIVATE ${CMAKE_SOURCE_DIR}/client/include)
target_link_libraries(r-type_client PRIVATE rtype_ecs rtype_shared ${SFML_LIBS})

#---------------------------------------------------------------------------
# Server Executable
#---------------------------------------------------------------------------
file(GLOB_RECURSE SERVER_SRC ${CMAKE_SOURCE_DIR}/server/src/*.cpp)

add_executable(r-type_server ${SERVER_SRC})
target_include_directories(r-type_server PRIVATE ${CMAKE_SOURCE_DIR}/server/include)
target_link_libraries(r-type_server PRIVATE rtype_ecs rtype_shared)

if(UNIX AND NOT APPLE)
    target_link_libraries(r-type_server PRIVATE pthread)
endif()

#---------------------------------------------------------------------------
# Tests
#---------------------------------------------------------------------------
find_package(Catch2 3 CONFIG REQUIRED)
enable_testing()
add_subdirectory(tests)
