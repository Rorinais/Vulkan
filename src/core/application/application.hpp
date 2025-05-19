#pragma once
#include "../../base.hpp"   
#include "../platform/window.hpp" 
#include "../utils/shaderUtils.hpp"
#include "../../rendering/core/commandPool.hpp"
#include "../../rendering/core/commandBuffer.hpp"
#include "../../rendering/core/semaphore.hpp"
#include "../../rendering/core/fence.hpp"
#include "../../rendering/vulkan/instance.hpp" 
#include "../../rendering/vulkan/vulkanDebug.hpp" 
#include "../../rendering/vulkan/physicalDevice.hpp" 
#include "../../rendering/vulkan/logicalDevice.hpp" 
#include "../../rendering/vulkan/swapchain.hpp"
#include"../../rendering/vulkan/renderPass.hpp"
#include"../../rendering/vulkan/pipeline.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

class Application {
public:
    struct FrameData {
        uint32_t imageIndex;
        VkResult acquireResult;
    };

    Application() = default;
    ~Application() = default;

    void run();

private:
    void initWindow();
    void initVulkan();
    void createSyncObjects();
    void createRenderPass();
    void createPipeline();
    void createCommandBuffers();
    void recreateSwapChain();
    void cleanupSwapChain();
    void drawFrame();
    void mainLoop();
    void cleanup();

    FrameData acquireNextImage();
    void submitCommandBuffer(uint32_t imageIndex);
    void presentFrame(uint32_t imageIndex);

private:
    Window::Ptr mWindow;
    Instance::Ptr mInstance;
    VulkanDebug::Ptr mDebug;
    PhysicalDevice::Ptr mPhysicalDevice;
    LogicalDevice::Ptr mLogicalDevice;
    SwapChain::Ptr mSwapChain;
    RenderPass::Ptr mRenderPass;
    Pipeline::Ptr mPipeline;
    CommandPool::Ptr mCommandPool;

    std::vector<CommandBuffer::Ptr> mCommandBuffers;
    std::vector<Semaphore::Ptr> mImageAvailableSemaphores;
    std::vector<Semaphore::Ptr> mRenderFinishedSemaphores;
    std::vector<Fence::Ptr> mInFlightFences;

    int mCurrentFrame = 0;
    std::atomic<bool> mFramebufferResized{ false };
};

