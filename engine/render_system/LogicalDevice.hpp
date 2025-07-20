#pragma once

#include "RenderBackend.hpp"
#include "BedrockSignal.hpp"
#include "ThreadSafeQueue.hpp"

#include <string>
#include <thread>
#include <vulkan/vulkan.h>

namespace MFA
{
    class LogicalDevice
    {
    public:
        
        struct InitParams
        {
            int windowWidth = 0;
            int windowHeight = 0;
            bool resizable = true;
            bool fullScreen = false;
            std::string applicationName {};
            // TODO: Maybe expose the sdl flags to support video and audio
        };

        [[nodiscard]]
        static std::unique_ptr<LogicalDevice> Init(InitParams const & params);

        explicit LogicalDevice(InitParams const & params);

        ~LogicalDevice();

        static void Update();

        static RT::CommandRecordState AcquireRecordState(VkSwapchainKHR swapChain);

        [[nodiscard]]
        static bool IsValid() noexcept;

        [[nodiscard]]
        static std::string ApplicationName() noexcept;

        [[nodiscard]]
        static bool IsResizable() noexcept;

        [[nodiscard]]
        static bool IsWindowVisible() noexcept;

        [[nodiscard]]
        static int GetWindowWidth() noexcept;

        [[nodiscard]]
        static int GetWindowHeight() noexcept;

        [[nodiscard]]
        static bool IsFullScreen() noexcept;

        [[nodiscard]]
        static VkInstance GetVkInstance() noexcept;

        [[nodiscard]]
        static VkSurfaceKHR GetSurface() noexcept;

        [[nodiscard]]
        static VkPhysicalDevice GetPhysicalDevice() noexcept;

        [[nodiscard]]
        static VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() noexcept;

        [[nodiscard]]
        static VkSampleCountFlagBits GetMaxSampleCount() noexcept;

        [[nodiscard]]
        static VkPhysicalDeviceProperties GetPhysicalDeviceProperties() noexcept;

        [[nodiscard]]
        static VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() noexcept;

        [[nodiscard]]
        static uint32_t GetSwapChainImageCount() noexcept;

        [[nodiscard]]
        static uint32_t GetMaxFramePerFlight() noexcept;

        [[nodiscard]]
        static uint32_t GetGraphicQueueFamily() noexcept;

        [[nodiscard]]
        static uint32_t GetComputeQueueFamily() noexcept;

        [[nodiscard]]
        static uint32_t GetPresentQueueFamily() noexcept;

        [[nodiscard]]
        static VkDevice GetVkDevice() noexcept;

        [[nodiscard]]
        static VkPhysicalDeviceMemoryProperties GetPhysicalMemoryProperties() noexcept;

        [[nodiscard]]
        static VkQueue GetGraphicQueue() noexcept;

        [[nodiscard]]
        static VkQueue GetComputeQueue() noexcept;

        [[nodiscard]]
        static VkQueue GetPresentQueue() noexcept;

        [[nodiscard]]
        static RT::CommandPoolGroup * GetGraphicCommandPool();

        [[nodiscard]]
        static std::vector<VkSemaphore> const& GetGraphicSemaphores() noexcept;

        [[nodiscard]]
        static std::vector<VkFence> const& GetGraphicFences() noexcept;

        [[nodiscard]]
        static RT::CommandPoolGroup * GetComputeCommandPool();

        [[nodiscard]]
        static std::vector<VkSemaphore> const & GetComputeSemaphores() noexcept;

        [[nodiscard]]
        static std::vector<VkSemaphore> const & GetPresentSemaphores() noexcept;

        [[nodiscard]]
        static VkFormat GetDepthFormat() noexcept;

        [[nodiscard]]
        static VkSurfaceFormatKHR GetSurfaceFormat() noexcept;

        [[nodiscard]]
        static VkSemaphore GetComputeSemaphore(RT::CommandRecordState const& recordState) noexcept;

        [[nodiscard]]
        static VkSemaphore GetPresentSemaphore(RT::CommandRecordState const& recordState) noexcept;

        [[nodiscard]]
        static VkFence GetFence(RT::CommandRecordState const& recordState);

        [[nodiscard]]
        static VkCommandBuffer GetComputeCommandBuffer(RT::CommandRecordState const& recordState);

        [[nodiscard]]
        static VkCommandBuffer GetGraphicCommandBuffer(RT::CommandRecordState const& recordState);

        static void BeginCommandBuffer(
            RT::CommandRecordState & recordState,
            RT::CommandBufferType commandBufferType,
            VkCommandBufferBeginInfo const & beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
            }
        );

        static void EndCommandBuffer(RT::CommandRecordState & recordState);

        static void SubmitQueues(RT::CommandRecordState & recordState);

        static void Present(RT::CommandRecordState const & recordState, VkSwapchainKHR swapChain);

        static void DeviceWaitIdle();

        [[nodiscard]]
        static SDL_Window* GetWindow() noexcept;

        using RenderTask = std::function<bool(RT::CommandRecordState & recordState)>;
        static void AddRenderTask(RenderTask renderTask);

    private:

        void OnResizeEvent(SDL_Event * event);

        void UpdateSurface();

        RT::CommandRecordState Internal_AcquireRecordState(VkSwapchainKHR swapChain);

        void Internal_Present(RT::CommandRecordState const & recordState, VkSwapchainKHR swapChain);

        void Internal_DeviceWaitIdle();

        void Internal_BeginCommandBuffer(
            RT::CommandRecordState & recordState,
            RT::CommandBufferType commandBufferType,
            VkCommandBufferBeginInfo const & beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
            }
        );

        void Internal_SubmitQueues(RT::CommandRecordState & recordState) const;

        [[nodiscard]]
        VkSemaphore Internal_GetComputeSemaphore(RT::CommandRecordState const& recordState) const noexcept;

        [[nodiscard]]
        VkSemaphore Internal_GetPresentSemaphore(RT::CommandRecordState const& recordState) const noexcept;

        [[nodiscard]]
        RT::CommandPoolGroup * Internal_GetGraphicCommandPool();

        [[nodiscard]]
        RT::CommandPoolGroup * Internal_GetComputeCommandPool();

        [[nodiscard]]
        VkCommandBuffer Internal_GetComputeCommandBuffer(RT::CommandRecordState const& recordState) const;

        [[nodiscard]]
        VkCommandBuffer Internal_GetGraphicCommandBuffer(RT::CommandRecordState const& recordState) const;

        static int SDLEventWatcher(void * data, SDL_Event * event);

    public:

        inline static Signal<> ResizeEventSignal1{};
        inline static Signal<> ResizeEventSignal2{};

        inline static Signal<SDL_Event*> SDL_EventSignal{};

    private:

        bool _isValid = false;

        std::string _applicationName {};
        SDL_Window * _window = nullptr;
        bool _resizable = false;
        bool _windowResized = true;
        bool _windowVisible = true;

        int _windowWidth {};
        int _windowHeight {};
        bool _fullScreen {};

        VkInstance _vkInstance {};

        VkSurfaceKHR _surface {};

        VkPhysicalDevice _physicalDevice {};
        VkPhysicalDeviceFeatures _physicalDeviceFeatures{};
        VkSampleCountFlagBits _maxSampleCount{};
        VkPhysicalDeviceProperties _physicalDeviceProperties{};

        VkSurfaceCapabilitiesKHR _surfaceCapabilities {};

        uint32_t _swapChainImageCount {};
        uint32_t _maxFramePerFlight {};
        uint32_t _currentFrame{};

        uint32_t _graphicQueueFamily {};
        uint32_t _computeQueueFamily {};
        uint32_t _presentQueueFamily {};

        VkDevice _vkDevice {};
        VkPhysicalDeviceMemoryProperties _physicalMemoryProperties{};

        VkQueue _graphicQueue {};
        VkQueue _computeQueue {};
        VkQueue _presentQueue {};

        std::unordered_map<std::thread::id, std::shared_ptr<RT::CommandPoolGroup>> _graphicCommandPoolMap {};
        std::shared_ptr<RT::CommandBufferGroup> _graphicCommandBuffer {};

        std::vector<VkFence> _fences {};

        std::unordered_map<std::thread::id, std::shared_ptr<RT::CommandPoolGroup>> _computeCommandPoolMap {};
        std::shared_ptr<RT::CommandBufferGroup> _computeCommandBuffer{};
        std::vector<VkSemaphore> _computeSemaphores {};

        std::vector<VkSemaphore> _presentSemaphores {};

        VkFormat _depthFormat {};
        VkSurfaceFormatKHR _surfaceFormat{};

        VkDebugReportCallbackEXT _vkDebugReportCallbackExt {};

        ThreadSafeQueue<RenderTask> _renderTasks{};
        std::queue<RenderTask> _pRenderTasks{};

        inline static LogicalDevice * _instance {};
    };

};