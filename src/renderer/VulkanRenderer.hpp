#pragma once
#include "../base.hpp"
#include "core/context/instance.hpp"
#include "core/context/vulkanDebug.hpp"
#include "core/context/physicalDevice.hpp"
#include "core/context/logicalDevice.hpp"
#include "core/context/swapchain.hpp"
#include "core/sync/fence.hpp"
#include "core/sync/semaphore.hpp"
#include "core/commands/commandPool.hpp"
#include "core/commands/commandBuffer.hpp"
#include "passes/renderPass.hpp"
#include "pipeline/pipelineStates/pipeline.hpp"

class VulkanRenderer {
public:

private:
    Instance::Ptr mInstance;
    VulkanDebug::Ptr mDebug;
    PhysicalDevice::Ptr mPhysicalDevice;
    LogicalDevice::Ptr mLogicalDevice;
    SwapChain::Ptr mSwapChain;
    RenderPass::Ptr mRenderPass;
    Pipeline::Ptr mCurrentPipeline;
    CommandPool::Ptr mCommandPool;

    std::vector<CommandBuffer::Ptr> mCommandBuffers;
    std::vector<Semaphore::Ptr> mImageAvailableSemaphores;
    std::vector<Semaphore::Ptr> mRenderFinishedSemaphores;
    std::vector<Fence::Ptr> mInFlightFences;

    uint32_t mCurrentFrame = 0;
    uint32_t mImageIndex;
};


