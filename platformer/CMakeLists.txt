add_executable(doodle_jump
    main.cpp
    ${CMAKE_SOURCE_DIR}/client/src/SFMLRenderer.cpp
    ${CMAKE_SOURCE_DIR}/client/src/AccessibilityManager.cpp
)

target_include_directories(doodle_jump PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/ecs/include
    ${CMAKE_SOURCE_DIR}/shared
)

target_link_libraries(doodle_jump PRIVATE
    rtype_ecs
    rtype_shared
    sfml-graphics sfml-window sfml-system sfml-audio
    OpenAL::OpenAL
)

if(UNIX)
    target_link_libraries(doodle_jump PRIVATE pthread Threads::Threads fmt::fmt)
endif()
