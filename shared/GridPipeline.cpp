#include "GridPipeline.hpp"

#include "BedrockAssert.hpp"
#include "BedrockPath.hpp"
#include "ImportShader.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

using namespace MFA;

//======================================================================================================================

GridPipeline::GridPipeline(VkRenderPass renderPass)
{
    mRenderPass = renderPass;
    CreatePipeline();
}

//======================================================================================================================

GridPipeline::~GridPipeline()
{
    mPipeline = nullptr;
}

//======================================================================================================================

bool GridPipeline::IsBinded(MFA::RT::CommandRecordState const &recordState) const
{
    if (recordState.pipeline == mPipeline.get())
    {
        return true;
    }
    return false;
}

//======================================================================================================================

void GridPipeline::BindPipeline(MFA::RT::CommandRecordState &recordState) const
{
    if (IsBinded(recordState))
    {
        return;
    }

    RB::BindPipeline(recordState, *mPipeline);
}

//======================================================================================================================

void GridPipeline::SetPushConstant(
    RT::CommandRecordState &recordState,
    PushConstants const &pushConstant
) const
{
    // TODO: These information are duplicate! We need to read from spirv
    RB::PushConstants(
        recordState,
        mPipeline->pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        Alias{pushConstant}
    );
}

//======================================================================================================================

void GridPipeline::Reload()
{
    CreatePipeline();
}

//======================================================================================================================

void GridPipeline::CreatePipeline()
{
    // Vertex shader
	{
		bool success = Importer::CompileShaderToSPV(
			Path::Get("shaders/grid_pipeline/GridPipeline.vert.hlsl"),
			Path::Get("shaders/grid_pipeline/GridPipeline.vert.spv"),
			"vert"
		);
		MFA_ASSERT(success == true);
	}
	auto cpuVertexShader = Importer::ShaderFromSPV(
		Path::Get("shaders/grid_pipeline/GridPipeline.vert.spv"),
		VK_SHADER_STAGE_VERTEX_BIT,
		"main"
	);
	auto gpuVertexShader = RB::CreateShader(
		LogicalDevice::GetVkDevice(),
		cpuVertexShader
	);

	// Fragment shader
	{
		bool success = Importer::CompileShaderToSPV(
			Path::Get("shaders/grid_pipeline/GridPipeline.frag.hlsl"),
			Path::Get("shaders/grid_pipeline/GridPipeline.frag.hlsl.spv"),
			"frag"
		);
		MFA_ASSERT(success == true);
	}
	auto cpuFragmentShader = Importer::ShaderFromSPV(
		Path::Get("shaders/grid_pipeline/GridPipeline.frag.hlsl.spv"),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		"main"
	);
	auto gpuFragmentShader = RB::CreateShader(
		LogicalDevice::GetVkDevice(),
		cpuFragmentShader
	);

	std::vector<RT::GpuShader const*> shaders{ gpuVertexShader.get(), gpuFragmentShader.get() };

	std::vector<VkVertexInputBindingDescription> const bindingDescriptions
	{
	    VkVertexInputBindingDescription
	    {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        }
	};

	std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
    // Vertex
	// Position
	inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
		.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(Vertex, position),
	});

	RB::CreateGraphicPipelineOptions pipelineOptions{};
	pipelineOptions.useStaticViewportAndScissor = false;
	pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineOptions.rasterizationSamples = LogicalDevice::GetMaxSampleCount();
	pipelineOptions.cullMode = VK_CULL_MODE_NONE;
	pipelineOptions.colorBlendAttachments.blendEnable = VK_FALSE;
	pipelineOptions.polygonMode = VK_POLYGON_MODE_FILL;

	// pipeline layout
	std::vector<VkPushConstantRange> const pushConstantRanges{
		VkPushConstantRange {
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.offset = 0,
			.size = sizeof(PushConstants),
		}
	};

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};

	const auto pipelineLayout = RB::CreatePipelineLayout(
		LogicalDevice::GetVkDevice(),
		static_cast<uint32_t>(descriptorSetLayouts.size()),
		descriptorSetLayouts.data(),
		static_cast<uint32_t>(pushConstantRanges.size()),
		pushConstantRanges.data()
	);

	auto const surfaceCapabilities = LogicalDevice::GetSurfaceCapabilities();

	mPipeline = RB::CreateGraphicPipeline(
		LogicalDevice::GetVkDevice(),
		static_cast<uint8_t>(shaders.size()),
		shaders.data(),
		bindingDescriptions.size(),
		bindingDescriptions.data(),
		static_cast<uint8_t>(inputAttributeDescriptions.size()),
		inputAttributeDescriptions.data(),
		surfaceCapabilities.currentExtent,
		mRenderPass,
		pipelineLayout,
		pipelineOptions
	);
}

//======================================================================================================================
