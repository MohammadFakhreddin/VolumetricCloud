#include "SceneRenderResource.hpp"

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

using namespace MFA;

//======================================================================================================================

SceneRenderResource::SceneRenderResource(
    VkExtent2D const imageExtent,
    VkFormat const imageFormat,
    VkFormat const depthFormat,
    VkSampleCountFlagBits const msaaSampleCount
)
    :_imageExtent(imageExtent)
    ,_imageFormat(imageFormat)
    ,_depthFormat(depthFormat)
    ,_msaaSampleCount(msaaSampleCount)
{
    _msaaSampleCount = LogicalDevice::GetMaxSampleCount();

    auto const maxImageCount = LogicalDevice::GetSwapChainImageCount();
    _msaaImageList.resize(maxImageCount);
    _colorImageList.resize(maxImageCount);
    _depthImageList.resize(maxImageCount);
    for (int i = 0; i < maxImageCount; ++i)
    {
        // Note: For some reason we can't have resolve image for non primary render passes
        {// MSAA Image
            RB::CreateColorImageOptions params{};
            params.samplesCount = _msaaSampleCount;
            params.imageType = VK_IMAGE_TYPE_2D;
            params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            _msaaImageList[i] = RB::CreateColorImage(
                LogicalDevice::GetPhysicalDevice(),
                LogicalDevice::GetVkDevice(),
                _imageExtent,
                _imageFormat,
                params
            );
        }

        {// Color Image
            RB::CreateColorImageOptions params{};
            params.samplesCount = VK_SAMPLE_COUNT_1_BIT;
            params.imageType = VK_IMAGE_TYPE_2D;
            params.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            _colorImageList[i] = RB::CreateColorImage(
                LogicalDevice::GetPhysicalDevice(),
                LogicalDevice::GetVkDevice(),
                _imageExtent,
                _imageFormat,
                params
            );
        }

        {// Depth Image
            _depthImageList[i] = RB::CreateDepthImage(
                LogicalDevice::GetPhysicalDevice(),
                LogicalDevice::GetVkDevice(),
                _imageExtent,
                _depthFormat,
                RB::CreateDepthImageOptions{
                    .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .samplesCount = _msaaSampleCount
                }
            );
        }
    }
}

//======================================================================================================================

SceneRenderResource::~SceneRenderResource() = default;

//======================================================================================================================
