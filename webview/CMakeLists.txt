set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/WebViewContainer.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/IShadingPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/TextOverlayPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/TextOverlayPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/SolidFillPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/SolidFillPipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/ImagePipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/ImagePipeline.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/BorderPipeline.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/BorderPipeline.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/CustomFontRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/CustomFontRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/SolidFillRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/SolidFillRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/ImageRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/ImageRenderer.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/BorderRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/BorderRenderer.hpp"
)

set(LIBRARY_NAME "Webview")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(${LIBRARY_NAME} litehtml)
target_link_libraries(${LIBRARY_NAME} glm)
target_link_libraries(${LIBRARY_NAME} SDL2-static)
target_link_libraries(${LIBRARY_NAME} RenderSystem)