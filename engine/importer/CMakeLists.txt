
set(LIBRARY_SOURCES)

list(
    APPEND LIBRARY_SOURCES

    "${CMAKE_CURRENT_SOURCE_DIR}/ImportShader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportShader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportTexture.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportTexture.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/ImportGLTF.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportGLTF.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/ImportObj.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportObj.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/JsonUtils.hpp"
)

set(LIBRARY_NAME "Importer")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})
target_link_libraries(${LIBRARY_NAME} glm)
target_link_libraries(${LIBRARY_NAME} LibConfig)
target_link_libraries(${LIBRARY_NAME} Vulkan::Vulkan)
target_link_libraries(${LIBRARY_NAME} Bedrock)


