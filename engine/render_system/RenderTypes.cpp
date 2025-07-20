#include "RenderTypes.hpp"

#include "BedrockAssert.hpp"
#include "RenderBackend.hpp"
#include "LogicalDevice.hpp"
#include "ScopeLock.hpp"
#include "Time.hpp"
#include "JobSystem.hpp"

#include <utility>

namespace MFA::RenderTypes
{
	//-------------------------------------------------------------------------------------------------

	ImageGroup::ImageGroup(
		VkImage image_,
		VkDeviceMemory memory_
	)
		: image(image_)
		, memory(memory_)
	{}

	//-------------------------------------------------------------------------------------------------

	ImageGroup::~ImageGroup()
	{
		RB::DestroyImage(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	ImageViewGroup::ImageViewGroup(VkImageView imageView_)
		: imageView(imageView_)
	{
		MFA_ASSERT(imageView != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	ImageViewGroup::~ImageViewGroup()
	{
		RB::DestroyImageView(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	DepthImageGroup::DepthImageGroup(
		std::shared_ptr<ImageGroup> imageGroup_,
		std::shared_ptr<ImageViewGroup> imageView_,
		VkFormat imageFormat_
	)
		: imageGroup(std::move(imageGroup_))
		, imageView(std::move(imageView_))
		, imageFormat(imageFormat_)
	{}

	//-------------------------------------------------------------------------------------------------

	DepthImageGroup::~DepthImageGroup() = default;

	//-------------------------------------------------------------------------------------------------

	DescriptorSetLayoutGroup::DescriptorSetLayoutGroup(VkDescriptorSetLayout descriptorSetLayout_)
		: descriptorSetLayout(descriptorSetLayout_)
	{
		MFA_ASSERT(descriptorSetLayout != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	DescriptorSetLayoutGroup::~DescriptorSetLayoutGroup()
    {
        RB::DestroyDescriptorSetLayout(LogicalDevice::GetVkDevice(), descriptorSetLayout);
    }

    //-------------------------------------------------------------------------------------------------

    // DescriptorSet::DescriptorSet(VkDescriptorSet descriptorSet_)
    //     : descriptorSet(descriptorSet_)
    // {}
    //
    // DescriptorSet::~DescriptorSet() = default;

    //-------------------------------------------------------------------------------------------------

	PipelineGroup::PipelineGroup(
		VkPipelineLayout pipelineLayout_,
		VkPipeline pipeline_
	)
		: pipelineLayout(pipelineLayout_)
		, pipeline(pipeline_)
	{
		MFA_ASSERT(IsValid());
	}

	//-------------------------------------------------------------------------------------------------

	PipelineGroup::~PipelineGroup()
	{
		RB::DestroyPipeline(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	bool PipelineGroup::IsValid() const noexcept
	{
		return pipelineLayout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE;
	}

	//-------------------------------------------------------------------------------------------------

	SwapChainGroup::SwapChainGroup(
		VkSwapchainKHR swapChain_,
		VkFormat swapChainFormat_,
		std::vector<VkImage> swapChainImages_,
		std::vector<std::shared_ptr<ImageViewGroup>> swapChainImageViews_
	)
		: swapChain(swapChain_)
		, swapChainFormat(swapChainFormat_)
		, swapChainImages(std::move(swapChainImages_))
		, swapChainImageViews(std::move(swapChainImageViews_))
	{}

	SwapChainGroup::~SwapChainGroup() { RB::DestroySwapChain(LogicalDevice::GetVkDevice(), *this); }

    //-------------------------------------------------------------------------------------------------

    CommandPoolGroup::~CommandPoolGroup()
	{
	    RB::DestroyCommandPool(LogicalDevice::GetVkDevice(), commandPool);
	}

    //-------------------------------------------------------------------------------------------------

    CommandBufferGroup::CommandBufferGroup(
        std::vector<VkCommandBuffer> commandBuffers_,
        CommandPoolGroup & commandPool_,
        std::thread::id const threadId_
    )
        : commandBuffers(std::move(commandBuffers_))
        , commandPool(commandPool_)
        , threadId(threadId_)
    {}

    CommandBufferGroup::~CommandBufferGroup()
	{
	    CommandPoolGroup * myCommandPool = &commandPool;
	    std::shared_ptr<int> counter = std::make_shared<int>(LogicalDevice::GetMaxFramePerFlight() + 1);
	    LogicalDevice::AddRenderTask([commandBuffers = commandBuffers, myCommandPool, counter](CommandRecordState const & recordState)->bool
        {
            (*counter) -= 1;
            if ((*counter) <= 0)
            {
                if (TryLock(myCommandPool->lock) == true)
                {
                    RB::DestroyCommandBuffers(
                        LogicalDevice::GetVkDevice(),
                        *myCommandPool,
                        commandBuffers.size(),
                        commandBuffers.data()
                    );
                    Unlock(myCommandPool->lock);
                    // Releasing the update
                    return false;
                }
            }
            return true;
        });
	}

    //-------------------------------------------------------------------------------------------------

	ColorImageGroup::ColorImageGroup(std::shared_ptr<ImageGroup> 
		imageGroup_,
		std::shared_ptr<ImageViewGroup> imageView_, 
		VkFormat imageFormat_
	)
		: imageGroup(std::move(imageGroup_))
		, imageView(std::move(imageView_))
		, imageFormat(imageFormat_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	ColorImageGroup::~ColorImageGroup() = default;

	//-------------------------------------------------------------------------------------------------

	RenderPass::RenderPass(VkRenderPass renderPass_)
		: vkRenderPass(renderPass_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	RenderPass::~RenderPass()
	{
		RB::DestroyRenderPass(LogicalDevice::GetVkDevice(), vkRenderPass);
	}

	//-------------------------------------------------------------------------------------------------

	FrameBuffer::FrameBuffer(VkFramebuffer framebuffer_)
		: framebuffer(framebuffer_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	FrameBuffer::~FrameBuffer()
	{
		RB::DestroyFrameBuffer(LogicalDevice::GetVkDevice(), framebuffer);
	}

	//-------------------------------------------------------------------------------------------------

	GpuShader::GpuShader(
		VkShaderModule const& shaderModule, 
		VkShaderStageFlagBits const stageFlags,
		std::string entryPointName
	)
		: shaderModule(shaderModule)
		, stageFlags(stageFlags)
		, entryPointName(std::move(entryPointName))
	{
		MFA_ASSERT(shaderModule != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	GpuShader::~GpuShader()
	{
		RB::DestroyShader(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	DescriptorPool::DescriptorPool(VkDescriptorPool descriptorPool_)
		: descriptorPool(descriptorPool_)
	{}

	//-------------------------------------------------------------------------------------------------

	DescriptorPool::~DescriptorPool()
	{
		RB::DestroyDescriptorPool(LogicalDevice::GetVkDevice(), descriptorPool);
	}

	//-------------------------------------------------------------------------------------------------

	BufferAndMemory::BufferAndMemory(VkBuffer buffer_, VkDeviceMemory memory_, VkDeviceSize size_)
		: buffer(buffer_)
		, memory(memory_)
		, size(size_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	BufferAndMemory::~BufferAndMemory()
	{
		RB::DestroyBuffer(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	BufferGroup::BufferGroup(
		std::vector<std::shared_ptr<BufferAndMemory>> buffers_,
		size_t bufferSize_,
		VkMemoryPropertyFlags memoryPropertyFlags_,
		VkBufferUsageFlags bufferUsageFlags_
	)
		: buffers(std::move(buffers_))
		, bufferSize(bufferSize_)
		, memoryPropertyFlags(memoryPropertyFlags_)
		, bufferUsageFlags(bufferUsageFlags_)
	{}

	//-------------------------------------------------------------------------------------------------

	BufferGroup::~BufferGroup() = default;

	//-------------------------------------------------------------------------------------------------

	SamplerGroup::SamplerGroup(VkSampler sampler_)
		: sampler(sampler_)
	{}

	//-------------------------------------------------------------------------------------------------

	SamplerGroup::~SamplerGroup()
	{
		RB::DestroySampler(LogicalDevice::GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	GpuTexture::GpuTexture() = default;

	//-------------------------------------------------------------------------------------------------

	GpuTexture::GpuTexture(
		std::shared_ptr<ImageGroup> imageGroup, 
		std::shared_ptr<ImageViewGroup> imageView
	)
		: imageGroup(std::move(imageGroup))
		, imageView(std::move(imageView))
	{}

	//-------------------------------------------------------------------------------------------------

	GpuTexture::~GpuTexture() = default;

	//-------------------------------------------------------------------------------------------------

}
