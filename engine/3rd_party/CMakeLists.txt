set(third_party_folder "ThirdParty")

if(NOT TARGET glm)
    option(BUILD_STATIC_LIBS "" ON)
    option(BUILD_TESTING "" OFF)
    option(GLM_TEST_ENABLE "" OFF)
    add_subdirectory(glm)
    set_target_properties(glm_static PROPERTIES FOLDER ${third_party_folder}/glm)
endif()

if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_folder}/glfw)
endif()

if(NOT TARGET glad)
    add_subdirectory(glad)
    set_target_properties(glad PROPERTIES FOLDER ${third_party_folder}/glad)
endif()

if(NOT TARGET stb)
    include(stb.cmake)
    set_target_properties(stb PROPERTIES FOLDER ${third_party_folder}/stb)
endif()

if(NOT TARGET assimp)
    file(GLOB assimp_sources CONFIGURE_DEPENDS  "${CMAKE_CURRENT_SOURCE_DIR}/assimp/*.h")
    add_library(assimp INTERFACE ${assimp_sources})
    target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assimp)
    set_target_properties(assimp PROPERTIES FOLDER ${third_party_folder}/assimp)
endif()


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
link_directories(${ASSIMP_LIBRARY})
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

file(GLOB DLLS "$(CMAKE_CURRENT_SOURCE_DIR)/dll/*.dll")






