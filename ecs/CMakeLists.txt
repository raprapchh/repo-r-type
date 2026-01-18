file(GLOB_RECURSE ECS_SRC CONFIGURE_DEPENDS src/*.cpp)
add_library(rtype_ecs ${ECS_SRC})

target_include_directories(rtype_ecs PUBLIC 
    include
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/client/include
)

target_link_libraries(rtype_ecs PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    OpenAL::OpenAL
    nlohmann_json::nlohmann_json
    fmt::fmt
)
