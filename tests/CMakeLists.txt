include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(unit_tests
    TestMovement.cpp
    TestNetworkSystem.cpp
    ${CMAKE_SOURCE_DIR}/client/src/NetworkSystem.cpp
    TestWeapon.cpp
)

target_link_libraries(unit_tests PRIVATE rtype_ecs rtype_shared Catch2::Catch2WithMain)
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/client/include
    ${CMAKE_SOURCE_DIR}/shared
    ${CMAKE_SOURCE_DIR}/ecs/include
)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
