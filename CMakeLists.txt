cmake_minimum_required(VERSION 3.20)
# ─────────────────────────────
# vcpkg Integration
# ─────────────────────────────
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(rtype LANGUAGES CXX)

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

# For multi-configuration generators (Visual Studio)
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
find_package(OpenAL CONFIG REQUIRED)

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
# Modules
# ─────────────────────────────
add_subdirectory(shared)
add_subdirectory(ecs)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(platformer)
