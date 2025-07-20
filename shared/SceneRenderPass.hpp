#pragma once

#include "BufferTracker.hpp"
#include "SceneFrameBuffer.hpp"

class SceneRenderPass
{
public:

    explicit SceneRenderPass(
        VkFormat imageFormat,
        VkFormat depthFormat,
        VkSampleCountFlagBits sampleCount
    );
    ~SceneRenderPass();

    // This is a special case so we don't need record state
    void Begin(MFA::RT::CommandRecordState const & recordState, SceneFrameBuffer const & frameBuffer) const;

    void End(MFA::RT::CommandRecordState const & recordState);

    [[nodiscard]]
    VkRenderPass GetRenderPass() const;

private:

    void CreateRenderPass();

    VkFormat _imageFormat;
    VkFormat _depthFormat;
    VkSampleCountFlagBits _sampleCount;
    std::unique_ptr<MFA::RT::RenderPass> _renderPass;

};
