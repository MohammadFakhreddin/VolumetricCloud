#include "SceneRenderPass.hpp"

#include "LogicalDevice.hpp"

#include <array>

// Use Offscreen rendering:
// https://github.com/SaschaWillems/Vulkan/blob/master/examples/offscreen/offscreen.cpp#L348

using namespace MFA;

//======================================================================================================================
// We need a renderResource per frame
SceneRenderPass::SceneRenderPass(
    VkFormat imageFormat,
    VkFormat depthFormat,
    VkSampleCountFlagBits sampleCount
)
{
    _imageFormat = imageFormat;
    _depthFormat = depthFormat;
    _sampleCount = sampleCount;
    CreateRenderPass();
}

//======================================================================================================================

SceneRenderPass::~SceneRenderPass() = default;

//======================================================================================================================

void SceneRenderPass::Begin(RT::CommandRecordState const & recordState, SceneFrameBuffer const & frameBuffer) const
{
    auto const & imageExtent = frameBuffer.ImageExtent();

    RB::AssignViewportAndScissorToCommandBuffer(imageExtent, recordState.commandBuffer);

    std::vector<VkClearValue> clearValues(3);
    clearValues[0].color = VkClearColorValue{ .float32 = {0.1f, 0.1f, 0.12f, 1.0f } };
    clearValues[1].color = VkClearColorValue{ .float32 = {0.1f, 0.1f, 0.12f, 1.0f } };
    clearValues[2].depthStencil = { .depth = 1.0f, .stencil = 0 };

    RB::BeginRenderPass(
        recordState.commandBuffer,
        _renderPass->vkRenderPass,
        frameBuffer.FrameIndex(recordState.imageIndex),
        imageExtent,
        static_cast<uint32_t>(clearValues.size()),
        clearValues.data()
    );
}

//======================================================================================================================

void SceneRenderPass::End(RT::CommandRecordState const & recordState)
{
    vkCmdEndRenderPass(recordState.commandBuffer);
}

//======================================================================================================================

VkRenderPass SceneRenderPass::GetRenderPass() const { return _renderPass->vkRenderPass; }

//======================================================================================================================

void SceneRenderPass::CreateRenderPass()
{
    // Multi-sampled attachment that we render to
    VkAttachmentDescription const msaaAttachment{
        .format = _imageFormat,
        .samples = _sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkAttachmentDescription const resolveAttachment{
        .format = _imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkAttachmentDescription const depthAttachment{
        .format = _depthFormat,
        .samples = _sampleCount,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Note: hardware will automatically transition attachment to the specified layout
    // Note: index refers to attachment descriptions array
    constexpr VkAttachmentReference msaaAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    constexpr VkAttachmentReference imageAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    constexpr VkAttachmentReference depthAttachmentReference{
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Note: this is a description of how the attachments of the render pass will be used in this sub pass
    // e.g. if they will be read in shaders and/or drawn to
    std::vector<VkSubpassDescription> subPassDescription{
        VkSubpassDescription {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &msaaAttachmentReference,
            .pResolveAttachments = &imageAttachmentReference,
            .pDepthStencilAttachment = &depthAttachmentReference,
        }
    };

    std::vector<VkAttachmentDescription> attachments = { msaaAttachment, resolveAttachment, depthAttachment };

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> subPassDependencies{};

    subPassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependencies[0].dstSubpass = 0;
    subPassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subPassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subPassDependencies[0].srcAccessMask = VK_ACCESS_NONE_KHR;
    subPassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subPassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subPassDependencies[1].srcSubpass = 0;
    subPassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subPassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subPassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subPassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subPassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    _renderPass = std::make_unique<RT::RenderPass>(RB::CreateRenderPass(
        LogicalDevice::GetVkDevice(),
        attachments.data(),
        static_cast<uint32_t>(attachments.size()),
        subPassDescription.data(),
        static_cast<uint32_t>(subPassDescription.size()),
        subPassDependencies.data(),
        subPassDependencies.size()
    ));
}

//======================================================================================================================
