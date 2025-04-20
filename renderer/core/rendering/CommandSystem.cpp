#include "CommandSystem.hpp"

CommandSystem::CommandSystem(
    VkDevice device,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkRenderPass renderPass,
    VkExtent2D swapChainExtent,
    uint32_t maxFramesInFlight 
) : mDevice(device),
mGraphicsQueue(graphicsQueue),
mPresentQueue(presentQueue),
mRenderPass(renderPass),
mSwapChainExtent(swapChainExtent),
mMaxFramesInFlight(maxFramesInFlight)
{
    mFrameContexts.resize(mMaxFramesInFlight);
    mImagesInFlight.clear();
}

CommandSystem::~CommandSystem() {
    cleanup();
}
void CommandSystem::cleanup() {
    // 确保先清理命令缓冲区
    cleanupCommandBuffers(); 

    // 销毁同步对象
    for (auto& ctx : mFrameContexts) {
        if (ctx.imageAvailable != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, ctx.imageAvailable, nullptr);
            ctx.imageAvailable = VK_NULL_HANDLE;
        }
        if (ctx.renderFinished != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, ctx.renderFinished, nullptr);
            ctx.renderFinished = VK_NULL_HANDLE;
        }
        if (ctx.inFlightFence != VK_NULL_HANDLE) {
            vkDestroyFence(mDevice, ctx.inFlightFence, nullptr);
            ctx.inFlightFence = VK_NULL_HANDLE;
        }
    }

    // 最后销毁命令池
    if (mCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        mCommandPool = VK_NULL_HANDLE;
    }
}

void CommandSystem::createCommandPool(QueueFamilyIndices queueFamilyIndices) {

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandSystem::createCommandBuffers(
    const std::vector<VkFramebuffer>& framebuffers,
    VkPipeline graphicsPipeline,
    VkBuffer& vertexBuffer,
    const uint32_t vertexCount,
    VkBuffer& indexBuffer,
    const uint32_t indexCount,
    const DescriptorSetCollection& descriptorSets, 
    VkPipelineLayout pipelineLayout){

    mPipelineLayout = pipelineLayout;

    // 参数验证加强
    if (framebuffers.empty()) {
        throw std::runtime_error("Cannot create command buffers with empty framebuffers!");
    }

    mCommandBuffers.resize(framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

    if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
        recordCommandBuffer(
            mCommandBuffers[i],
            framebuffers[i],
            graphicsPipeline,
            vertexBuffer,
            vertexCount,
            indexBuffer,
            indexCount,
            descriptorSets[i] 
        );
    }
}

void CommandSystem::recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    VkFramebuffer framebuffer,
    VkPipeline graphicsPipeline,
    VkBuffer& vertexBuffer,
    const uint32_t vertexCount,
    VkBuffer& indexBuffer,
    const uint32_t indexCount,
    const DescriptorSetMap& descriptorSets // 改为接收描述符集Map
) const
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapChainExtent;

    VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 绑定图形管线
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

   
    std::vector<uint32_t> setIndices;
    for (const auto& pair : descriptorSets) {
        setIndices.push_back(pair.first);
    }
    std::sort(setIndices.begin(), setIndices.end());

    std::vector<VkDescriptorSet> orderedSets;
    for (auto setIndex : setIndices) {
        orderedSets.push_back(descriptorSets.at(setIndex));
    }

    if (!orderedSets.empty()) {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mPipelineLayout,
            0, 
            static_cast<uint32_t>(orderedSets.size()),
            orderedSets.data(),
            0,
            nullptr
        );
    }
    

#if DYNAMIC_STATE
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = USE_OPENGL_COORDINATES
            ?static_cast<float>(mSwapChainExtent.height):0.0f; // 原点始终在左上角
        viewport.width = static_cast<float>(mSwapChainExtent.width);
        viewport.height = USE_OPENGL_COORDINATES
            ? -static_cast<float>(mSwapChainExtent.height) // 高度为负，Y轴向上
            : static_cast<float>(mSwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = mSwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
#endif 

        // 绑定顶点/索引缓冲区
        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // 绘制命令
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
}

void CommandSystem::recreateCommandBuffers(
    const std::vector<VkFramebuffer>& framebuffers,
    VkPipeline graphicsPipeline,
    VkExtent2D newExtent,
    VkBuffer& vertexBuffer,
    const uint32_t vertexCount,
    VkBuffer& indexBuffer,
    const uint32_t indexCount,
    const DescriptorSetCollection& descriptorSets){
    mSwapChainExtent = newExtent;

    if (!mCommandBuffers.empty()) {
        vkFreeCommandBuffers(
            mDevice,
            mCommandPool,
            static_cast<uint32_t>(mCommandBuffers.size()),
            mCommandBuffers.data()
        );
    }

    createCommandBuffers(
        framebuffers,
        graphicsPipeline,
        vertexBuffer,
        vertexCount,
        indexBuffer,
        indexCount,
        descriptorSets,
        mPipelineLayout
    );
}

void CommandSystem::cleanupCommandBuffers() {
    if (mCommandPool != VK_NULL_HANDLE && !mCommandBuffers.empty()) {
        // 仅在存在有效命令缓冲区时释放
        vkFreeCommandBuffers(
            mDevice,
            mCommandPool,
            static_cast<uint32_t>(mCommandBuffers.size()),
            mCommandBuffers.data()
        );
        mCommandBuffers.clear();
    }
}

void CommandSystem::createSyncObjects(uint32_t swapChainImageCount) {
    mImagesInFlight.resize(swapChainImageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; 

    for (auto& ctx : mFrameContexts) {
        // 创建信号量
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &ctx.imageAvailable) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &ctx.renderFinished) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphores!");
        }

        // 创建 Fence
        if (vkCreateFence(mDevice, &fenceInfo, nullptr, &ctx.inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence!");
        }
    }
}

void CommandSystem::updateSyncObjects(size_t imageCount) {
    destroySyncObjects(); 
    createSyncObjects(static_cast<uint32_t>(imageCount)); 
}

void CommandSystem::destroySyncObjects() {
    vkDeviceWaitIdle(mDevice);

    for (auto& ctx : mFrameContexts) {
        if (ctx.imageAvailable != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, ctx.imageAvailable, nullptr);
            ctx.imageAvailable = VK_NULL_HANDLE;
        }
        if (ctx.renderFinished != VK_NULL_HANDLE) {
            vkDestroySemaphore(mDevice, ctx.renderFinished, nullptr);
            ctx.renderFinished = VK_NULL_HANDLE;
        }
        if (ctx.inFlightFence != VK_NULL_HANDLE) {
            vkDestroyFence(mDevice, ctx.inFlightFence, nullptr);
            ctx.inFlightFence = VK_NULL_HANDLE;
        }
    }
    mImagesInFlight.clear();
}

bool CommandSystem::beginFrame(VkSwapchainKHR swapChain, uint32_t& imageIndex, bool& needRecreate) {
    auto& ctx = mFrameContexts[mCurrentFrame];
    needRecreate = false;

    // 检查 Fence 有效性
    if (ctx.inFlightFence == VK_NULL_HANDLE) {
        throw std::runtime_error("Invalid fence handle!");
    }

    vkWaitForFences(mDevice, 1, &ctx.inFlightFence, VK_TRUE, UINT64_MAX);

    // 获取交换链图像索引
    VkResult result = vkAcquireNextImageKHR(
        mDevice, swapChain, UINT64_MAX,
        ctx.imageAvailable, VK_NULL_HANDLE, &imageIndex
    );

    // 错误处理
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        needRecreate = (result == VK_ERROR_OUT_OF_DATE_KHR);
        return false;
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    // 检查 imageIndex 有效性
    if (imageIndex >= mImagesInFlight.size()) {
        throw std::runtime_error("Acquired image index out of sync objects range!");
    }

    // 等待该图像关联的前一个操作完成
    if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(mDevice, 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    mImagesInFlight[imageIndex] = ctx.inFlightFence;

    return true;
}

void CommandSystem::submitFrame(uint32_t imageIndex) {
    if (imageIndex >= mCommandBuffers.size()) {
        throw std::runtime_error("Invalid command buffer index!");
    }
    auto& ctx = mFrameContexts[mCurrentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { ctx.imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { ctx.renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice, 1, &ctx.inFlightFence);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, ctx.inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
}

bool CommandSystem::presentFrame(VkSwapchainKHR swapChain, uint32_t imageIndex, bool& needRecreate) {
    auto& ctx = mFrameContexts[mCurrentFrame];

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &ctx.renderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

    mCurrentFrame = (mCurrentFrame + 1) % mMaxFramesInFlight;

    needRecreate = (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR);
    return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
}