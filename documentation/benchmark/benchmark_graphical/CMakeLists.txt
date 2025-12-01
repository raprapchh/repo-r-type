# Benchmark Graphique - CMakeLists.txt
# Ce fichier permet de compiler les benchmarks avec CMake

cmake_minimum_required(VERSION 3.20)

# SFML Benchmark
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SFML QUIET sfml-graphics sfml-window sfml-system)
    if(SFML_FOUND)
        add_executable(bench_sfml bench_sfml.cpp)
        target_link_libraries(bench_sfml ${SFML_LIBRARIES})
        target_include_directories(bench_sfml PRIVATE ${SFML_INCLUDE_DIRS})
        target_compile_definitions(bench_sfml PRIVATE ${SFML_CFLAGS_OTHER})
        message(STATUS "SFML benchmark configuré")
    else()
        message(WARNING "SFML non trouvé - benchmark SFML ignoré")
    endif()

    # Raylib Benchmark
    pkg_check_modules(RAYLIB QUIET raylib)
    if(RAYLIB_FOUND)
        add_executable(bench_raylib bench_raylib.cpp)
        target_link_libraries(bench_raylib ${RAYLIB_LIBRARIES})
        target_include_directories(bench_raylib PRIVATE ${RAYLIB_INCLUDE_DIRS})
        target_compile_definitions(bench_raylib PRIVATE ${RAYLIB_CFLAGS_OTHER})
        message(STATUS "Raylib benchmark configuré")
    else()
        message(WARNING "Raylib non trouvé - benchmark Raylib ignoré")
    endif()

    # SDL2 Benchmark
    pkg_check_modules(SDL2 QUIET sdl2 SDL2_image)
    if(SDL2_FOUND)
        add_executable(bench_sdl2 bench_sdl2.cpp)
        target_link_libraries(bench_sdl2 ${SDL2_LIBRARIES})
        target_include_directories(bench_sdl2 PRIVATE ${SDL2_INCLUDE_DIRS})
        target_compile_definitions(bench_sdl2 PRIVATE ${SDL2_CFLAGS_OTHER})
        message(STATUS "SDL2 benchmark configuré")
    else()
        message(WARNING "SDL2 non trouvé - benchmark SDL2 ignoré")
    endif()
else()
    message(WARNING "PkgConfig non trouvé - utiliser test.sh à la place")
endif()

# Target pour lancer tous les benchmarks
add_custom_target(run_graphics_benchmark
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Exécution du benchmark graphique"
    VERBATIM
)
