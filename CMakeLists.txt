cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(ASIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/asio/asio/include")

if(NOT EXISTS "${ASIO_INCLUDE_DIR}/asio.hpp")
    message(STATUS "ASIO not found. Downloading...")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/chriskohlhoff/asio.git ${CMAKE_SOURCE_DIR}/external/asio
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone ASIO repository")
    endif()
    message(STATUS "ASIO downloaded successfully")
else()
    message(STATUS "ASIO found at ${ASIO_INCLUDE_DIR}")
endif()

include_directories(${ASIO_INCLUDE_DIR})

#
# ECS
#
file(GLOB_RECURSE ECS_SRC
    ${CMAKE_SOURCE_DIR}/ecs/src/*.cpp
)

add_library(rtype_ecs STATIC ${ECS_SRC})
target_include_directories(rtype_ecs PUBLIC
    ${CMAKE_SOURCE_DIR}/ecs/include
)

#
# SHARED
#
add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE
    ${CMAKE_SOURCE_DIR}/shared
)

#
# CLIENT
#
file(GLOB_RECURSE CLIENT_SRC
    ${CMAKE_SOURCE_DIR}/client/src/*.cpp
)

add_executable(r-type_client ${CLIENT_SRC})
target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
)
target_link_libraries(r-type_client
    PRIVATE rtype_ecs rtype_shared
)

#
# SERVER
#
file(GLOB_RECURSE SERVER_SRC
    ${CMAKE_SOURCE_DIR}/server/src/*.cpp
)

add_executable(r-type_server ${SERVER_SRC})
target_include_directories(r-type_server PRIVATE
    ${CMAKE_SOURCE_DIR}/server/include
)
target_link_libraries(r-type_server
    PRIVATE rtype_ecs rtype_shared
)

if(UNIX AND NOT APPLE)
    target_link_libraries(r-type_server PRIVATE pthread)
endif()
