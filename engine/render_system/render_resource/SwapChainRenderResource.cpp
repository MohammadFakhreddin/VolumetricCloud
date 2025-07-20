#include "SwapChainRenderResource.hpp"

#include "../RenderBackend.hpp"
#include "../LogicalDevice.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    SwapChainRenderResource::SwapChainRenderResource()
    {
        mSwapChainImages = RB::CreateSwapChain(
			LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            LogicalDevice::GetSurface(),
            LogicalDevice::GetSurfaceCapabilities(),
            LogicalDevice::GetSurfaceFormat(),
            nullptr
        );
    }

    //-------------------------------------------------------------------------------------------------

    SwapChainRenderResource::~SwapChainRenderResource()
    {
        mSwapChainImages.reset();
    }

    //-------------------------------------------------------------------------------------------------

    VkImage SwapChainRenderResource::GetSwapChainImage(RT::CommandRecordState const& recordState) const
    {
        return mSwapChainImages->swapChainImages[recordState.imageIndex];
    }

    //-------------------------------------------------------------------------------------------------

    RT::SwapChainGroup const& SwapChainRenderResource::GetSwapChainImages() const
    {
        return *mSwapChainImages;
    }

    //-------------------------------------------------------------------------------------------------

    void SwapChainRenderResource::OnResize()
    {
        // Swap-chain
        auto const oldSwapChainImages = mSwapChainImages;
        mSwapChainImages = RB::CreateSwapChain(
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            LogicalDevice::GetSurface(),
            LogicalDevice::GetSurfaceCapabilities(),
            LogicalDevice::GetSurfaceFormat(),
            oldSwapChainImages->swapChain
        );
    }

    //-------------------------------------------------------------------------------------------------

}
