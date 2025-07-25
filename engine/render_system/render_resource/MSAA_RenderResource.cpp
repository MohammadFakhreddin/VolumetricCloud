#include "MSAA_RenderResource.hpp"

#include "../LogicalDevice.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    MSSAA_RenderResource::MSSAA_RenderResource()
        : RenderResource()
    {
        CreateMSAAImage();
    }

    //-------------------------------------------------------------------------------------------------

    MSSAA_RenderResource::~MSSAA_RenderResource()
    {
        mMSAAImageGroupList.clear();
    }

    //-------------------------------------------------------------------------------------------------

    RT::ColorImageGroup const& MSSAA_RenderResource::GetImageGroup(int const index) const
    {
        return *mMSAAImageGroupList[index];
    }

    //-------------------------------------------------------------------------------------------------

    void MSSAA_RenderResource::OnResize()
    {
        mMSAAImageGroupList.clear();
        CreateMSAAImage();
    }

    //-------------------------------------------------------------------------------------------------

    void MSSAA_RenderResource::CreateMSAAImage()
    {
        auto surfaceCapabilities = LogicalDevice::GetSurfaceCapabilities();
        auto const swapChainExtend = VkExtent2D{
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height
        };

        mMSAAImageGroupList.resize(LogicalDevice::GetSwapChainImageCount());
        for (auto & MSAA_Image : mMSAAImageGroupList)
        {
            MSAA_Image = RB::CreateColorImage(
                LogicalDevice::GetPhysicalDevice(),
                LogicalDevice::GetVkDevice(),
                swapChainExtend,
                LogicalDevice::GetSurfaceFormat().format,
                RB::CreateColorImageOptions{
                    .samplesCount = LogicalDevice::GetMaxSampleCount()
                }
            );
        }
    }

    //-------------------------------------------------------------------------------------------------

}
