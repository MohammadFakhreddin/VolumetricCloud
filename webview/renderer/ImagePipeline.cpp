#include "ImagePipeline.hpp"

#include "BedrockPath.hpp"
#include "DescriptorSetSchema.hpp"
#include "ImportShader.hpp"
#include "LogicalDevice.hpp"

using namespace MFA;

//-------------------------------------------------------------------------------------------------

ImagePipeline::ImagePipeline(
    std::shared_ptr<DisplayRenderPass> displayRenderPass,
    std::shared_ptr<RT::SamplerGroup> sampler
)
    : _displayRenderPass(std::move(displayRenderPass))
    , _sampler(std::move(sampler))
{
    _descriptorPool = RB::CreateDescriptorPool(LogicalDevice::GetVkDevice(), 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    CreateDescriptorLayout();
    CreatePipeline();
}

//-------------------------------------------------------------------------------------------------

ImagePipeline::~ImagePipeline()
{
    _pipeline = nullptr;
    _descriptorLayout = nullptr;
    _descriptorPool = nullptr;
}

//-------------------------------------------------------------------------------------------------

bool ImagePipeline::IsBinded(RT::CommandRecordState const &recordState) const
{
    if (recordState.pipeline == _pipeline.get())
    {
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::BindPipeline(RT::CommandRecordState &recordState) const
{
    if (IsBinded(recordState))
    {
        return;
    }

    RB::BindPipeline(recordState, *_pipeline);
}

//-------------------------------------------------------------------------------------------------

RT::DescriptorSetGroup ImagePipeline::CreateDescriptorSet(RT::GpuTexture const & texture) const
{
    auto descriptorSetGroup = RB::CreateDescriptorSet(
        LogicalDevice::GetVkDevice(),
        _descriptorPool->descriptorPool,
        _descriptorLayout->descriptorSetLayout,
        1
    );

    UpdateDescriptorSet(descriptorSetGroup, texture);

    return descriptorSetGroup;
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::UpdateDescriptorSet(RT::DescriptorSetGroup &descriptorSetGroup, RT::GpuTexture const &texture) const
{
    MFA_ASSERT(descriptorSetGroup.descriptorSets.size() == 1);

    auto const &descriptorSet = descriptorSetGroup.descriptorSets[0];
    MFA_ASSERT(descriptorSet != VK_NULL_HANDLE);

    DescriptorSetSchema schema{descriptorSet};

    VkDescriptorImageInfo info{.sampler = _sampler->sampler,
                               .imageView = texture.imageView->imageView,
                               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    schema.AddCombinedImageSampler(&info);

    schema.UpdateDescriptorSets();
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::FreeDescriptorSet(RT::DescriptorSetGroup &descriptorSetGroup) const
{
    vkFreeDescriptorSets(
        LogicalDevice::GetVkDevice(),
        _descriptorPool->descriptorPool,
        descriptorSetGroup.descriptorSets.size(),
        descriptorSetGroup.descriptorSets.data()
    );
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::SetPushConstant(RT::CommandRecordState &recordState, PushConstants const &pushConstant) const
{
    RB::PushConstants(
        recordState,
        _pipeline->pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0, Alias{pushConstant}
    );
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::Reload()
{
    CreatePipeline();
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::CreateDescriptorLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings{};

    // Sampler
    VkDescriptorSetLayoutBinding const combinedImageSampler{
        .binding = static_cast<uint32_t>(bindings.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    bindings.emplace_back(combinedImageSampler);

    _descriptorLayout = RB::CreateDescriptorSetLayout(
        LogicalDevice::GetVkDevice(),
        static_cast<uint8_t>(bindings.size()), bindings.data()
    );
}

//-------------------------------------------------------------------------------------------------

void ImagePipeline::CreatePipeline()
{
    // Vertex shader
    {
        bool success = Importer::CompileShaderToSPV(
            Path::Get("shaders/image_pipeline/ImagePipeline.vert.hlsl"),
            Path::Get("shaders/image_pipeline/ImagePipeline.vert.spv"),
            "vert"
        );
        MFA_ASSERT(success == true);
    }
    auto cpuVertexShader = Importer::ShaderFromSPV(
        Path::Get("shaders/image_pipeline/ImagePipeline.vert.spv"),
        VK_SHADER_STAGE_VERTEX_BIT,
        "main"
    );
    auto gpuVertexShader = RB::CreateShader(LogicalDevice::GetVkDevice(), cpuVertexShader);

    // Fragment shader
    {
        bool success = Importer::CompileShaderToSPV(
            Path::Get("shaders/image_pipeline/ImagePipeline.frag.hlsl"),
            Path::Get("shaders/image_pipeline/ImagePipeline.frag.spv"),
            "frag"
        );
        MFA_ASSERT(success == true);
    }
    auto cpuFragmentShader = Importer::ShaderFromSPV(
        Path::Get("shaders/image_pipeline/ImagePipeline.frag.spv"),
        VK_SHADER_STAGE_FRAGMENT_BIT,
        "main"
    );
    auto gpuFragmentShader = RB::CreateShader(LogicalDevice::GetVkDevice(), cpuFragmentShader);

    std::vector<RT::GpuShader const *> shaders{gpuVertexShader.get(), gpuFragmentShader.get()};

	std::vector<VkVertexInputBindingDescription> const bindingDescriptions{
    VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    },
    VkVertexInputBindingDescription{
        .binding = 1,
        .stride = sizeof(Instance),
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
    }};

    std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
    // Position
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        0,
        Vertex,
        position
    ));
    // UV
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        0,
        Vertex,
        uv
    ));
    // topLeftPos
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        topLeftPos
    ));
    // topLeftRadius
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        topLeftRadius
    ));
    // bottomLeftPos
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        bottomLeftPos
    ));
    // bottomLeftRadius
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        bottomLeftRadius
    ));
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        topRightPos
    ));
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        topRightRadius
    ));
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        bottomRightPos
    ));
    inputAttributeDescriptions.emplace_back(MFA_VERTEX_INPUT_ATTRIBUTE(
        static_cast<uint32_t>(inputAttributeDescriptions.size()),
        1,
        Instance,
        bottomRightRadius
    ));

    RB::CreateGraphicPipelineOptions pipelineOptions{};
    pipelineOptions.useStaticViewportAndScissor = false;
    pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    pipelineOptions.rasterizationSamples = LogicalDevice::GetMaxSampleCount();
    pipelineOptions.cullMode = VK_CULL_MODE_NONE;
    pipelineOptions.colorBlendAttachments.blendEnable = VK_TRUE;
    pipelineOptions.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineOptions.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineOptions.depthStencil.depthTestEnable = false;
    pipelineOptions.depthStencil.depthWriteEnable = false;

    std::vector<VkPushConstantRange> pushConstantRanges{};
    {
        pushConstantRanges.emplace_back();
        auto &pushConstant = pushConstantRanges.back();
        pushConstant.size = sizeof(PushConstants);
        pushConstant.offset = 0;
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    }

    // pipeline layout
    std::vector<VkDescriptorSetLayout> setLayout{_descriptorLayout->descriptorSetLayout};

    const auto pipelineLayout = RB::CreatePipelineLayout(
        LogicalDevice::GetVkDevice(),
        setLayout.size(),
        setLayout.data(),
        pushConstantRanges.size(),
        pushConstantRanges.data()
    );

    auto surfaceCapabilities = LogicalDevice::GetSurfaceCapabilities();

    _pipeline = RB::CreateGraphicPipeline(
        LogicalDevice::GetVkDevice(),
        static_cast<uint8_t>(shaders.size()),
        shaders.data(),
        bindingDescriptions.size(),
        bindingDescriptions.data(),
        static_cast<uint8_t>(inputAttributeDescriptions.size()),
        inputAttributeDescriptions.data(),
        surfaceCapabilities.currentExtent,
        _displayRenderPass->GetVkRenderPass(),
        pipelineLayout,
        pipelineOptions
    );
}

//-------------------------------------------------------------------------------------------------
