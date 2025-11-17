cmake_minimum_required(VERSION 3.20)

project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
if(CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${cfg}" cfg_upper)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg_upper} ${CMAKE_SOURCE_DIR})
    endforeach()
endif()

file(GLOB_RECURSE R_TYPE_CLIENT_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/client/src/*.cpp)
file(GLOB_RECURSE R_TYPE_SERVER_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/server/src/*.cpp)

add_library(rtype_ecs INTERFACE)
target_include_directories(rtype_ecs INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs/include)

add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_executable(r-type_client ${R_TYPE_CLIENT_SOURCES})
target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/client/include)
target_link_libraries(r-type_client PRIVATE rtype_ecs rtype_shared)

add_executable(r-type_server ${R_TYPE_SERVER_SOURCES})
target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/server/include)
target_link_libraries(r-type_server PRIVATE rtype_ecs rtype_shared)
