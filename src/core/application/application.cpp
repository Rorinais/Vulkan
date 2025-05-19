#include "application.hpp"
#include <stdexcept>
#include <array>

void Application::initWindow() {
    Window::Config config{
        .width = 800,
        .height = 600,
        .title = "Vulkan Demo",
        .resizable = true,
        .monitorIndex = 0,
        .fullScreen = false,
        .highDPI = false
    };
    mWindow = Window::create(config);

    mWindow->setKeyCallback([this](int key, int action) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(mWindow->getHandle(), GLFW_TRUE);
        }});

    mWindow->setResizeCallback([this](int, int) {
        mFramebufferResized.store(true);
        });
}

void Application::initVulkan() {
    mInstance = Instance::create({ "Vulkan App" });
    mDebug = VulkanDebug::create(mInstance);

    auto surface = mWindow->createSurface(mInstance->getHandle());
    mPhysicalDevice = PhysicalDevice::create(mInstance, surface);

    LogicalDevice::Config deviceConfig{};
    mLogicalDevice = LogicalDevice::create(mPhysicalDevice, deviceConfig);

    mSwapChain = SwapChain::create(mLogicalDevice, mWindow);

    createRenderPass();
    createPipeline();
    createCommandBuffers();
    createSyncObjects();
}

void Application::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(mWindow->getHandle())) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(mLogicalDevice->getHandle());
}

void Application::cleanup() {
    cleanupSwapChain();
    mCommandPool.reset();
    mLogicalDevice.reset();
    mPhysicalDevice.reset();
    mDebug.reset();
    mInstance.reset();
    mWindow.reset();
}

void Application::createRenderPass() {
    mRenderPass = RenderPass::create(mLogicalDevice->getHandle());
    mRenderPass->addAttachment({
        RenderPass::Type::Color,
        mSwapChain->getSwapChainImageFormat(),
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        }).build();
}

void Application::createPipeline() {
    mPipeline = Pipeline::create(mLogicalDevice);

    mPipeline->setRenderPass(mRenderPass);
    mPipeline->setVertexInputState(VertexInput{});
    mPipeline->setViewportState(Viewport().IsOpenglCoordinate().createViewport(mSwapChain->getSwapChainExtent()));
    mPipeline->setPipelineLayout(PipelineLayout::create(mLogicalDevice, {}));

    mPipeline->setShaderStage([this] {
        ShaderStages::Ptr shaderStages = ShaderStages::create(mLogicalDevice);
        shaderStages->addGLSLStage("assets/shaders/core/shader.vert", VK_SHADER_STAGE_VERTEX_BIT, "main", {}, "VertexShader");
        shaderStages->addGLSLStage("assets/shaders/core/shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT, "main", {}, "FragmentShader");
        return shaderStages;
        }());

    mPipeline->createGraphicsPipeline();
    mSwapChain->createFramebuffers(mRenderPass->getHandle());
}

void Application::createCommandBuffers() {
    auto framebuffers = mSwapChain->getSwapChainFramebuffers();
    auto extent = mSwapChain->getSwapChainExtent();
    mCommandPool = CommandPool::create(mLogicalDevice);
    mCommandBuffers.resize(framebuffers.size());

    for (size_t i = 0; i < framebuffers.size(); ++i) {
        mCommandBuffers[i] = CommandBuffer::create(mLogicalDevice, mCommandPool);
        mCommandBuffers[i]->begin();

        std::vector<VkClearValue> clearValues{};
        VkClearValue clearColor;
        clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues.push_back(clearColor);
        //clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass->getHandle();
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        mCommandBuffers[i]->beginRenderPass(renderPassInfo);
        mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getHandle());
        mCommandBuffers[i]->setViewport(extent);
        mCommandBuffers[i]->setScissor(extent);
        mCommandBuffers[i]->draw(3);
        mCommandBuffers[i]->endRenderPass();
        mCommandBuffers[i]->end();
    }
}

void Application::recreateSwapChain() {
    vkDeviceWaitIdle(mLogicalDevice->getHandle());
    cleanupSwapChain();
    mSwapChain = SwapChain::create(mLogicalDevice, mWindow);
    createRenderPass();
    createPipeline();
    createCommandBuffers();
}

void Application::cleanupSwapChain() {
    mCommandBuffers.clear();
    mPipeline.reset();
    mRenderPass.reset();
    mSwapChain.reset();
}

void Application::createSyncObjects() {
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mImageAvailableSemaphores[i] = Semaphore::create(mLogicalDevice);
        mRenderFinishedSemaphores[i] = Semaphore::create(mLogicalDevice);
        mInFlightFences[i] = Fence::create(mLogicalDevice, true);
    }
}

void Application::drawFrame() {
    FrameData frameData = acquireNextImage();

    if (frameData.acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }else if (frameData.acquireResult != VK_SUCCESS && frameData.acquireResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    submitCommandBuffer(frameData.imageIndex);

    presentFrame(frameData.imageIndex);
}

Application::FrameData Application::acquireNextImage() {
    FrameData result{};

    mInFlightFences[mCurrentFrame]->block();

    result.acquireResult = vkAcquireNextImageKHR(
        mLogicalDevice->getHandle(),
        mSwapChain->getHandle(),
        UINT64_MAX,
        mImageAvailableSemaphores[mCurrentFrame]->getHandle(),
        VK_NULL_HANDLE,
        &result.imageIndex
    );
    return result;
}

void Application::submitCommandBuffer(uint32_t imageIndex) {
    mInFlightFences[mCurrentFrame]->resetFence();

    VkCommandBuffer commandBuffers[] = { mCommandBuffers[imageIndex]->getHandle() };
    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getHandle() };
    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getHandle() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mLogicalDevice->getQueueHandles().graphicsQueue,1, &submitInfo,mInFlightFences[mCurrentFrame]->getHandle()) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void Application::presentFrame(uint32_t imageIndex) {
    VkSwapchainKHR swapChains[] = { mSwapChain->getHandle() };
    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getHandle() };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult presentResult = vkQueuePresentKHR(mLogicalDevice->getQueueHandles().presentQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR ||presentResult == VK_SUBOPTIMAL_KHR ||mFramebufferResized) {
        mFramebufferResized = false;
        recreateSwapChain();
    }else if (presentResult != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

