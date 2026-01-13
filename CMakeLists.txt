cmake_minimum_required(VERSION 3.20)
project(rtype LANGUAGES CXX)

# ─────────────────────────────
# vcpkg Integration
# ─────────────────────────────
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(ASIO_STANDALONE)

# Warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Platform Detection & Output Directories
if (WIN32)
    set(PLATFORM_NAME "windows")
    add_compile_definitions(RTYPE_WINDOWS)
else()
    set(PLATFORM_NAME "linux")
    add_compile_definitions(RTYPE_LINUX)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})

# For multi-configuration generators (Visual Studio) ensure all configs output
# to the same `bin/${PLATFORM_NAME}` directory so CI copy-step finds the exe
if (CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UP)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UP} ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UP} ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UP} ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_NAME})
    endforeach()
endif()

# ─────────────────────────────
# Dependencies (vcpkg)
# ─────────────────────────────
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt CONFIG REQUIRED)

# ─────────────────────────────
# SFML 2.6.2 (via FetchContent - self-contained)
# ─────────────────────────────
include(FetchContent)
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

# Catch2
find_package(Catch2 CONFIG QUIET)
if(Catch2_FOUND OR TARGET Catch2::Catch2WithMain)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Catch2 not found, tests disabled")
endif()

# ─────────────────────────────
# Libraries
# ─────────────────────────────

# ECS
file(GLOB_RECURSE ECS_SRC CONFIGURE_DEPENDS ecs/src/*.cpp)
add_library(rtype_ecs ${ECS_SRC})
target_include_directories(rtype_ecs PUBLIC ecs/include)
target_link_libraries(rtype_ecs PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Shared
file(GLOB_RECURSE SHARED_SRC CONFIGURE_DEPENDS shared/*.cpp)
add_library(rtype_shared STATIC ${SHARED_SRC})
target_include_directories(rtype_shared PUBLIC shared)

# ─────────────────────────────
# Client executable
# ─────────────────────────────
file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS client/src/*.cpp)
add_executable(r-type_client ${CLIENT_SRC})
target_include_directories(r-type_client PRIVATE client/include)
target_link_libraries(r-type_client PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    Threads::Threads
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    fmt::fmt
)

# ─────────────────────────────
# Server executable
# ─────────────────────────────
file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS server/src/*.cpp)
add_executable(r-type_server ${SERVER_SRC})
target_include_directories(r-type_server PRIVATE server/include)
target_link_libraries(r-type_server PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    Threads::Threads
    fmt::fmt
)



# Windows specifics
if (WIN32)
    target_compile_definitions(r-type_client PRIVATE WIN32_LEAN_AND_MEAN)
    target_compile_definitions(r-type_server PRIVATE WIN32_LEAN_AND_MEAN)
    # Link sfml-main if it exists
    if(TARGET SFML::Main)
        target_link_libraries(r-type_client PRIVATE SFML::Main)
    elseif(TARGET sfml-main)
         target_link_libraries(r-type_client PRIVATE sfml-main)
    endif()
endif()
