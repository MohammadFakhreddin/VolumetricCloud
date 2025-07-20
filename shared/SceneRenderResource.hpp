#pragma once

#include "RenderTypes.hpp"

class SceneRenderResource
{
public:

    explicit SceneRenderResource(
        VkExtent2D imageExtent,
        VkFormat imageFormat,
        VkFormat depthFormat,
        VkSampleCountFlagBits msaaSampleCount
    );
    ~SceneRenderResource();

    [[nodiscard]]
    VkExtent2D const & ImageExtent() const {return _imageExtent;}

    [[nodiscard]]
    VkFormat ImageFormat() const {return _imageFormat;}

    [[nodiscard]]
    VkSampleCountFlagBits MSAA_SampleCount() const {return _msaaSampleCount;}

    [[nodiscard]]
    MFA::RT::ColorImageGroup const & MSAA_Image(MFA::RT::CommandRecordState const & recordState) const
    {
        return *_msaaImageList[recordState.imageIndex];
    }

    [[nodiscard]]
    MFA::RT::ColorImageGroup const & MSAA_Image(int const imageIndex) const
    {
        return *_msaaImageList[imageIndex];
    }

    [[nodiscard]]
    MFA::RT::ColorImageGroup const & ColorImage(MFA::RT::CommandRecordState const & recordState) const
    {
        return *_colorImageList[recordState.imageIndex];
    }

    [[nodiscard]]
    MFA::RT::ColorImageGroup const & ColorImage(int const imageIndex) const
    {
        return *_colorImageList[imageIndex];
    }

    [[nodiscard]]
    MFA::RT::DepthImageGroup const & DepthImage(MFA::RT::CommandRecordState const & recordState) const
    {
        return *_depthImageList[recordState.imageIndex];
    }

    [[nodiscard]]
    MFA::RT::DepthImageGroup const & DepthImage(int const imageIndex) const
    {
        return *_depthImageList[imageIndex];
    }

private:

    VkExtent2D const _imageExtent;
    VkFormat const _imageFormat;
    VkFormat const _depthFormat;
    VkSampleCountFlagBits _msaaSampleCount;

    std::vector<std::shared_ptr<MFA::RT::ColorImageGroup>> _msaaImageList;
    std::vector<std::shared_ptr<MFA::RT::ColorImageGroup>> _colorImageList;
    std::vector<std::shared_ptr<MFA::RT::DepthImageGroup>> _depthImageList;

};
