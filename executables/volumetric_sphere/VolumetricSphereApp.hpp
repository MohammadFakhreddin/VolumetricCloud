#pragma once

#include "RenderTypes.hpp"
#include "SceneRenderPass.hpp"
#include "GridRenderer.hpp"
#include "Time.hpp"
#include "UI.hpp"
#include "camera/ArcballCamera.hpp"

#include <SDL_events.h>

class VolumetricSphereApp
{
public:

    explicit VolumetricSphereApp();

    ~VolumetricSphereApp();

    void Run();

private:

    void Update(float deltaTime);

    void Render(MFA::RT::CommandRecordState & recordState);

    void Resize();

    void Reload();

    void OnSDL_Event(SDL_Event* event);

    void OnUI(float deltaTimeSec);

    void PrepareSceneRenderPass();

    void ApplyUI_Style();

    void DisplayParametersWindow();

    // You need to be able to select and view objects in the editor window
    void DisplaySceneWindow();

    std::shared_ptr<MFA::UI> _ui{};
    std::unique_ptr<MFA::Time> _time{};
    std::shared_ptr<MFA::SwapChainRenderResource> _swapChainResource{};
    std::shared_ptr<MFA::DepthRenderResource> _depthResource{};
    std::shared_ptr<MFA::MSSAA_RenderResource> _msaaResource{};
    std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};
    std::shared_ptr<MFA::RT::SamplerGroup> _sampler{};

    std::shared_ptr<SceneFrameBuffer> _sceneFrameBuffer{};
    std::shared_ptr<SceneRenderResource> _sceneRenderResource{};
    std::shared_ptr<SceneRenderPass> _sceneRenderPass{};

    std::vector<ImTextureID> _sceneTextureID_List{};
    VkExtent2D _sceneWindowSize{800, 800};
    bool _sceneWindowResized = false;
    bool _sceneWindowFocused = false;

    ImFont* _defaultFont{};
    ImFont* _boldFont{};

    std::shared_ptr<GridPipeline> _gridPipeline{};
    std::unique_ptr<GridRenderer> _gridRenderer{};

    std::unique_ptr<MFA::ArcballCamera> _camera{};

    int _activeImageIndex{};

    glm::vec3 _lightDirection = glm::vec3(-1.0f, 0.0f, -1.0f);
    glm::vec3 _lightColor {1.0f, 1.0f, 1.0f};
    float _lightIntensity = 1.0f;
    float _specularLightIntensity = 1.0f;
    int _shininess = 32;
    float _ambientStrength = 0.25f;
};
