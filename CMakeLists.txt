cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(ASIO_STANDALONE)
add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})


# EnTT (via vcpkg)
find_package(EnTT CONFIG REQUIRED)

# Asio (via vcpkg)
find_package(asio CONFIG REQUIRED)

# SFML (via vcpkg) - using SFML 2.6.x
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND)
    enable_testing()
    add_subdirectory(tests)
else()
    message(WARNING "Catch2 not found (missing CONFIG). Tests will be skipped.")
endif()


# --- LIBRARIES ---

# 1. ECS Library
file(GLOB_RECURSE ECS_SRC ${CMAKE_SOURCE_DIR}/ecs/src/*.cpp)
add_library(rtype_ecs ${ECS_SRC})
target_include_directories(rtype_ecs PUBLIC ${CMAKE_SOURCE_DIR}/ecs/include)
target_link_libraries(rtype_ecs PUBLIC EnTT::EnTT sfml-graphics sfml-window sfml-system sfml-audio)

# 2. Shared Library
add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE ${CMAKE_SOURCE_DIR}/shared)

# --- EXECUTABLES ---

# 3. Client
file(GLOB_RECURSE CLIENT_SRC ${CMAKE_SOURCE_DIR}/client/src/*.cpp)
add_executable(r-type_client ${CLIENT_SRC})
target_include_directories(r-type_client PRIVATE ${CMAKE_SOURCE_DIR}/client/include)
target_link_libraries(r-type_client PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    sfml-graphics sfml-window sfml-system sfml-audio
)

# 4. Server
file(GLOB_RECURSE SERVER_SRC ${CMAKE_SOURCE_DIR}/server/src/*.cpp)
add_executable(r-type_server ${SERVER_SRC})
target_include_directories(r-type_server PRIVATE ${CMAKE_SOURCE_DIR}/server/include)
target_link_libraries(r-type_server PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    EnTT::EnTT
)

if(UNIX)
    target_link_libraries(r-type_server PRIVATE pthread)
    target_link_libraries(r-type_client PRIVATE pthread)
endif()
