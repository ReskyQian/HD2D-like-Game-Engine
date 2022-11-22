file(GLOB assimp_sources CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/assimp/*.h")
add_library(assimp INTERFACE ${assimp_sources})
target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assimp)