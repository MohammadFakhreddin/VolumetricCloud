#pragma once

#include "RenderTypes.hpp"

#include <memory>
#include <vector>

#include "SceneRenderResource.hpp"

class SceneFrameBuffer
{
public:

    explicit SceneFrameBuffer(
        std::shared_ptr<SceneRenderResource> renderResource,
        VkRenderPass renderPass
    );
    ~SceneFrameBuffer();

    VkFramebuffer FrameIndex(int index) const;

    VkExtent2D ImageExtent() const;

private:

    std::shared_ptr<SceneRenderResource> _renderResource;
    std::vector<std::unique_ptr<MFA::RT::FrameBuffer>> _frameBufferList;

};
