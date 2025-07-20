#include "SceneFrameBuffer.hpp"

#include "LogicalDevice.hpp"

using namespace MFA;

//======================================================================================================================

SceneFrameBuffer::SceneFrameBuffer(std::shared_ptr<SceneRenderResource> renderResource, VkRenderPass renderPass)
    : _renderResource(std::move(renderResource))
{
    auto const & extent = _renderResource->ImageExtent();

    auto const swapChainExtent = VkExtent2D{
        .width = extent.width,
        .height = extent.height
    };

    _frameBufferList.resize(MFA::LogicalDevice::GetSwapChainImageCount());

    for (int imageIndex = 0; imageIndex < _frameBufferList.size(); imageIndex++)
    {
        auto const & msaaImage = _renderResource->MSAA_Image(imageIndex);
        auto const & colorImage = _renderResource->ColorImage(imageIndex);
        auto const & depthImage = _renderResource->DepthImage(imageIndex);

        std::vector<VkImageView> const attachments{
            msaaImage.imageView->imageView,
            colorImage.imageView->imageView,
            depthImage.imageView->imageView
        };
        // We only need one framebuffer
        _frameBufferList[imageIndex] = std::make_unique<RT::FrameBuffer>(
            RB::CreateFrameBuffers(
                MFA::LogicalDevice::GetVkDevice(),
                renderPass,
                attachments.data(),
                static_cast<uint32_t>(attachments.size()),
                swapChainExtent,
                1
            )
        );
    }
}

//======================================================================================================================

SceneFrameBuffer::~SceneFrameBuffer() = default;

//======================================================================================================================

VkFramebuffer SceneFrameBuffer::FrameIndex(int const index) const
{
    return _frameBufferList[index]->framebuffer;
}

//======================================================================================================================

VkExtent2D SceneFrameBuffer::ImageExtent() const
{
    return _renderResource->ImageExtent();
}

//======================================================================================================================
