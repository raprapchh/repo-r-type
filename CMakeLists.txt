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

# ─────────────────────────────
# Dependencies (vcpkg)
# ─────────────────────────────
find_package(asio CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)

# Defensive SFML search
message(STATUS "Searching for SFML...")
find_package(SFML CONFIG)
if(NOT SFML_FOUND)
    message(STATUS "SFML CONFIG not found, trying without CONFIG...")
    find_package(SFML COMPONENTS graphics window system audio REQUIRED)
else()
    message(STATUS "SFML CONFIG found version ${SFML_VERSION}")
endif()

# Robust target mapping
set(SFML_COMPONENTS Graphics Window System Audio)
foreach(comp ${SFML_COMPONENTS})
    string(TOLOWER ${comp} comp_lc)
    if(NOT TARGET SFML::${comp})
        if(TARGET SFML::${comp_lc})
            message(STATUS "Mapping SFML::${comp_lc} to SFML::${comp}")
            add_library(SFML::${comp} ALIAS SFML::${comp_lc})
        elseif(TARGET sfml-${comp_lc})
            message(STATUS "Mapping sfml-${comp_lc} to SFML::${comp}")
            add_library(SFML::${comp} ALIAS sfml-${comp_lc})
        else()
            message(WARNING "Could not find target for SFML ${comp}")
        endif()
    endif()
endforeach()

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
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    EnTT::EnTT
    nlohmann_json::nlohmann_json
)

# Shared
add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE shared)

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
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
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
