file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS src/*.cpp)

add_executable(r-type_server ${SERVER_SRC})

target_include_directories(r-type_server PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/ecs/include
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/client/include
)

target_link_libraries(r-type_server PRIVATE
    rtype_ecs
    rtype_shared
    asio::asio
    Threads::Threads
    OpenAL::OpenAL
    fmt::fmt
)

if(UNIX)
    target_link_libraries(r-type_server PRIVATE pthread)
endif()

if (WIN32)
    target_compile_definitions(r-type_server PRIVATE WIN32_LEAN_AND_MEAN)
endif()
