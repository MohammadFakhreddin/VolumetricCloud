#pragma once

#include "RenderTypes.hpp"
#include "pipeline/IShadingPipeline.hpp"

#include <glm/glm.hpp>

class GridPipeline : public MFA::IShadingPipeline
{
public:

    using Position = glm::vec3;
    using Color = glm::vec3;
    using Thickness = float;

    struct Vertex
    {
        Position position{};
    };

    struct PushConstants
    {
        glm::mat4 viewProjMat{};
    };

    explicit GridPipeline(VkRenderPass renderPass);

    ~GridPipeline();

    [[nodiscard]]
    bool IsBinded(MFA::RT::CommandRecordState const& recordState) const;

    void BindPipeline(MFA::RT::CommandRecordState& recordState) const;

    void SetPushConstant(
        MFA::RT::CommandRecordState &recordState,
        PushConstants const &pushConstant
    ) const;

    void Reload() override;

private:

    void CreatePipeline();

private:

    std::shared_ptr<MFA::RT::PipelineGroup> mPipeline{};
    VkRenderPass mRenderPass{};

};
