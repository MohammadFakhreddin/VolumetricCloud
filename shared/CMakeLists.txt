set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderPass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneRenderResource.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneFrameBuffer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SceneFrameBuffer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/GridRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Buffers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Buffers.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ShapeGenerator.hpp"
)

set(LIBRARY_NAME "Shared")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(${LIBRARY_NAME} SDL2-static)
target_link_libraries(${LIBRARY_NAME} glm)
target_link_libraries(${LIBRARY_NAME} Importer)