########################################

set(EXECUTABLE "VolumetricSphere")

list(
    APPEND EXECUTABLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/VolumetricSphereMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VolumetricSphereApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/VolumetricSphereApp.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})

target_link_libraries(${EXECUTABLE} glm)
target_link_libraries(${EXECUTABLE} Vulkan::Vulkan)
target_link_libraries(${EXECUTABLE} SDL2-static)
target_link_libraries(${EXECUTABLE} LibConfig)
target_link_libraries(${EXECUTABLE} Imgui)
target_link_libraries(${EXECUTABLE} Implot)
target_link_libraries(${EXECUTABLE} Bedrock)
target_link_libraries(${EXECUTABLE} AssetSystem)
target_link_libraries(${EXECUTABLE} EntitySystem)
target_link_libraries(${EXECUTABLE} Importer)
target_link_libraries(${EXECUTABLE} TimeSystem)
target_link_libraries(${EXECUTABLE} JobSystem)
target_link_libraries(${EXECUTABLE} RenderSystem)
target_link_libraries(${EXECUTABLE} Shared)
target_link_libraries(${EXECUTABLE} Webview)

########################################