cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

# --- CONFIGURATION ---

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(ASIO_STANDALONE)
add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# --- DEPENDENCIES ---

include(FetchContent)

# Asio (via vcpkg)
find_package(asio CONFIG REQUIRED)

# SFML 2.6.2 (via FetchContent - self-contained)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2
    GIT_SHALLOW    TRUE
)
set(SFML_BUILD_AUDIO TRUE CACHE BOOL "" FORCE)
set(SFML_BUILD_GRAPHICS TRUE CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW TRUE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# Catch2 (via vcpkg) - optional
find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND)
    enable_testing()
    add_subdirectory(tests)
else()
    message(WARNING "Catch2 not found. Tests will be skipped.")
endif()

# --- LIBRARIES ---

# 1. ECS Library
file(GLOB_RECURSE ECS_SRC ${CMAKE_SOURCE_DIR}/ecs/src/*.cpp)
add_library(rtype_ecs ${ECS_SRC})
target_include_directories(rtype_ecs PUBLIC ${CMAKE_SOURCE_DIR}/ecs/include)
target_link_libraries(rtype_ecs PUBLIC sfml-graphics sfml-window sfml-system sfml-audio)

# 2. Shared Library
file(GLOB_RECURSE SHARED_SRC ${CMAKE_SOURCE_DIR}/shared/*.cpp)
if(SHARED_SRC)
    add_library(rtype_shared ${SHARED_SRC})
    target_include_directories(rtype_shared PUBLIC ${CMAKE_SOURCE_DIR}/shared)
else()
    add_library(rtype_shared INTERFACE)
    target_include_directories(rtype_shared INTERFACE ${CMAKE_SOURCE_DIR}/shared)
endif()

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
    rtype_shared
    asio::asio
)

# 5. Platformer PoC
add_executable(platformer_game
    platformer/main.cpp
    client/src/SFMLRenderer.cpp
    client/src/AccessibilityManager.cpp
)
target_include_directories(platformer_game PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/ecs/include
    ${CMAKE_SOURCE_DIR}/shared
)
target_link_libraries(platformer_game PRIVATE
    rtype_ecs
    rtype_shared
    sfml-graphics sfml-window sfml-system sfml-audio
)

if(UNIX)
    target_link_libraries(r-type_server PRIVATE pthread)
    target_link_libraries(r-type_client PRIVATE pthread)
    target_link_libraries(platformer_game PRIVATE pthread)
endif()
