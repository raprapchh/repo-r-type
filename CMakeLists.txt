cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_ROOT "${CMAKE_SOURCE_DIR}/vcpkg")
    if(NOT EXISTS "${VCPKG_ROOT}")
        message(STATUS "VCPKG not found. Cloning...")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT}
            RESULT_VARIABLE GIT_RESULT
        )
        if(NOT GIT_RESULT EQUAL "0")
            message(FATAL_ERROR "Failed to clone VCPKG")
        endif()
        
        message(STATUS "Bootstrapping VCPKG...")
        if(WIN32)
            execute_process(COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.bat" WORKING_DIRECTORY "${VCPKG_ROOT}")
        else()
            execute_process(COMMAND "./bootstrap-vcpkg.sh" WORKING_DIRECTORY "${VCPKG_ROOT}")
        endif()
    endif()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(asio CONFIG REQUIRED)
include_directories(${asio_INCLUDE_DIRS})

find_package(EnTT REQUIRED)

#
# ECS
#
file(GLOB_RECURSE ECS_SRC
    ${CMAKE_SOURCE_DIR}/ecs/src/*.cpp
)

add_library(rtype_ecs ${ECS_SRC})
target_include_directories(rtype_ecs 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/ecs/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/ecs/include
)
target_link_libraries(rtype_ecs PUBLIC EnTT::EnTT)

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
