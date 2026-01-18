file(GLOB_RECURSE SHARED_SRC CONFIGURE_DEPENDS *.cpp)
add_library(rtype_shared STATIC ${SHARED_SRC})

target_include_directories(rtype_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
