file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS src/*.cpp)

add_executable(r-type_client ${CLIENT_SRC})

target_include_directories(r-type_client PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/ecs/include
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/server/include
)

target_link_libraries(r-type_client PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    Threads::Threads
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    OpenAL::OpenAL
    fmt::fmt
)

if(UNIX)
    target_link_libraries(r-type_client PRIVATE pthread)
endif()

if (WIN32)
    target_compile_definitions(r-type_client PRIVATE WIN32_LEAN_AND_MEAN)
    if(TARGET SFML::Main)
        target_link_libraries(r-type_client PRIVATE SFML::Main)
    elseif(TARGET sfml-main)
         target_link_libraries(r-type_client PRIVATE sfml-main)
    endif()
endif()
