#include "ImageRenderer.hpp"

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

namespace MFA
{

    //------------------------------------------------------------------------------------------------------------------

    ImageRenderer::ImageRenderer(std::shared_ptr<Pipeline> pipeline)
        : _pipeline(std::move(pipeline))
    {}

    //------------------------------------------------------------------------------------------------------------------

    std::unique_ptr<ImageRenderer::ImageData> ImageRenderer::AllocateImageData(
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
    ) const
    {
        Pipeline::Vertex vertexData[4]
        {
            Pipeline::Vertex{.position = topLeftPos, .radius = topLeftBorderRadius,.uv = topLeftUV},
            Pipeline::Vertex{.position = bottomLeftPos, .radius = bottomLeftBorderRadius, .uv = bottomLeftUV},
            Pipeline::Vertex{.position = topRightPos, .radius = topRightBorderRadius, .uv = topRightUV},
            Pipeline::Vertex{.position = bottomRightPos, .radius = bottomRightBorderRadius, .uv = bottomRightUV},
        };

        auto const vertexBuffer = RB::CreateVertexBufferGroup(
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            sizeof(vertexData),
            (int)LogicalDevice::GetMaxFramePerFlight()
        );

        auto const vertexStageBuffer = RB::CreateStageBuffer(
            LogicalDevice::GetVkDevice(),
            LogicalDevice::GetPhysicalDevice(),
            vertexBuffer->bufferSize,
            vertexBuffer->buffers.size()
        );

        auto imageData = std::make_unique<ImageData>(ImageData{
            .vertexData = LocalBufferTracker(vertexBuffer, vertexStageBuffer, Alias(vertexData)),
            .descriptorSet = _pipeline->CreateDescriptorSet(gpuTexture)
        });

        return imageData;
    }

    //------------------------------------------------------------------------------------------------------------------

    void ImageRenderer::UpdateImageData(
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
    ) const
    {
        MFA_ASSERT(imageData.vertexData.has_value() == true);
        auto * rawData = imageData.vertexData->Data();

        Pipeline::Vertex * vertexData = reinterpret_cast<Pipeline::Vertex *>(rawData);

        vertexData[0].position = topLeftPos;
        vertexData[0].radius = topLeftBorderRadius;
        vertexData[0].uv = topLeftUV;

        vertexData[1].position = bottomLeftPos;
        vertexData[1].radius = bottomLeftBorderRadius;
        vertexData[1].uv = bottomLeftUV;

        vertexData[2].position = topRightPos;
        vertexData[2].radius = topRightBorderRadius;
        vertexData[2].uv = topRightUV;

        vertexData[3].position = bottomRightPos;
        vertexData[3].radius = bottomRightBorderRadius;
        vertexData[3].uv = bottomRightUV;

        // TODO: We need to wrap descriptor sets as well to be freeable
        _pipeline->UpdateDescriptorSet(imageData.descriptorSet, gpuTexture);
    }

    //------------------------------------------------------------------------------------------------------------------

    void ImageRenderer::FreeImageData(ImageData &imageData)
    {
        imageData.vertexData.reset();
        _pipeline->FreeDescriptorSet(imageData.descriptorSet);
    }

    //------------------------------------------------------------------------------------------------------------------
    // TODO: I need to use viewport and scissor to render within area.
    void ImageRenderer::Draw(
        RT::CommandRecordState & recordState,
        Pipeline::PushConstants const & pushConstants,
        ImageData const & imageData
    ) const
    {
        _pipeline->BindPipeline(recordState);

        _pipeline->SetPushConstant(recordState, pushConstants);

        RB::AutoBindDescriptorSet(recordState, RB::UpdateFrequency::PerPipeline, imageData.descriptorSet);

    	auto const &localBuffers = imageData.vertexData->LocalBuffer().buffers;
        auto const &localBuffer = localBuffers[recordState.frameIndex % localBuffers.size()];
        VkBuffer buffers[2]{localBuffer->buffer, localBuffer->buffer};
        VkDeviceSize bindingOffsets[2]{0, 0};

        vkCmdBindVertexBuffers(recordState.commandBuffer, 0, 2, buffers, bindingOffsets);

        vkCmdDraw(recordState.commandBuffer, 4, 1, 0, 0);
    }

    //------------------------------------------------------------------------------------------------------------------

}
