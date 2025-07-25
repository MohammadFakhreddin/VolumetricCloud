#pragma once

#include "BufferTracker.hpp"
#include "ImagePipeline.hpp"

#include <optional>

namespace MFA
{
    class ImageRenderer
    {
    public:

        using Pipeline = ImagePipeline;
        using Position = Pipeline::Position;
		using UV = Pipeline::UV;
		using Radius = Pipeline::Radius;

        struct ImageData
        {
            std::optional<LocalBufferTracker> vertexData = std::nullopt;
            RT::DescriptorSetGroup descriptorSet;
        };

        explicit ImageRenderer(std::shared_ptr<Pipeline> pipeline);

        [[nodiscard]]
        std::unique_ptr<ImageData> AllocateImageData(
            RT::GpuTexture const & gpuTexture,

            Position const &topLeftPos,
            Position const &bottomLeftPos,
            Position const &topRightPos,
            Position const &bottomRightPos,

            Radius const &topLeftBorderRadius,
            Radius const &bottomLeftBorderRadius,
            Radius const &topRightBorderRadius,
            Radius const &bottomRightBorderRadius,

            UV const &topLeftUV,
            UV const &bottomLeftUV,
            UV const &topRightUV,
            UV const &bottomRightUV
        ) const;

        void UpdateImageData(
            ImageData &imageData,

            RT::GpuTexture const &gpuTexture,

            Position const &topLeftPos,
            Position const &bottomLeftPos,
            Position const &topRightPos,
            Position const &bottomRightPos,

            Radius const &topLeftBorderRadius,
            Radius const &bottomLeftBorderRadius,
            Radius const &topRightBorderRadius,
            Radius const &bottomRightBorderRadius,

            UV const &topLeftUV,
            UV const &bottomLeftUV,
            UV const &topRightUV,
            UV const &bottomRightUV
        ) const;

        void FreeImageData(ImageData &imageData);

        void Draw(
            RT::CommandRecordState & recordState,
            Pipeline::PushConstants const & pushConstants,
            ImageData const & imageData
        ) const;

    private:

        std::shared_ptr<Pipeline> _pipeline;

    };
}
