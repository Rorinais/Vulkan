#include "application.hpp"
#include <stdexcept>
#include <array>

struct UniformBufferObject {
    glm::mat4 mvp;
};

struct UniformBuffers {
    alignas(16) glm::mat4 model;

    alignas(16) glm::mat4 view;

    alignas(16) glm::mat4 proj;
};

const int GRID_SIZE = 100;  // 网格大小（单位数）
const int GRID_STEP = 1;    // 网格线间隔
const float GRID_FADE_DISTANCE = 50.0f; // 网格淡出距离

std::vector<glm::vec3> positions; 
std::vector<glm::vec3> colors;   
std::vector<uint32_t> indices;

void Application::initWindow() {
    Window::Config config{
        .width = 1280,
        .height = 720,
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
        mFramebufferResized = true;
        });
}

void Application::initVulkan() {
    mInstance = Instance::create({ "Vulkan App" });
    mDebug = VulkanDebug::create(mInstance);

    auto surface = mWindow->createSurface(mInstance->getHandle());
    mPhysicalDevice = PhysicalDevice::create(mInstance, surface);

    LogicalDevice::Config deviceConfig{};
    deviceConfig.samplerAnisotropy = VK_TRUE;
    deviceConfig.fillModeNonSolid = VK_TRUE;
    deviceConfig.wideLines = VK_TRUE;
    mLogicalDevice = LogicalDevice::create(mPhysicalDevice, deviceConfig);

    mSwapChain = SwapChain::create(mLogicalDevice, mWindow);

    mCommandPool = CommandPool::create(mLogicalDevice);

    createRenderPass();
    mSwapChain->createFramebuffers(mRenderPass->getHandle(),mCommandPool);

    createGeometry();
    mVertexBuffer = VertexArrayBuffer::create(mLogicalDevice, mCommandPool);
    mVertexBuffer->beginBinding(0);
    mVertexBuffer->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, positions);
    mVertexBuffer->finishBinding();

    mVertexBuffer->beginBinding(1);
    mVertexBuffer->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, colors);
    mVertexBuffer->finishBinding();

    //mVertexBuffer->beginBinding(2);
    //mVertexBuffer->addAttribute(2, VK_FORMAT_R32G32B32_SFLOAT, normals);
    //mVertexBuffer->finishBinding();


    mIndexBuffer =IndexBuffer::create(mLogicalDevice, mCommandPool);
    mIndexBuffer->loadData(indices);

    //mDescriptorManager = new UniformBufferManager(mLogicalDevice,mCommandPool);
    //mDescriptorManager->addUniformBinding<UniformBuffers>(0, 0, VK_SHADER_STAGE_VERTEX_BIT);
    //mDescriptorManager->addTextureBinding(1, 0, VK_SHADER_STAGE_FRAGMENT_BIT, "C:\\Users\\41384\\Desktop\\luguan.jpg");
    //mDescriptorManager->createDescriptorResources(mSwapChain->getSwapChainFramebuffers().size());

    mDescriptorManager = new UniformBufferManager(mLogicalDevice,mCommandPool);
    mDescriptorManager->addUniformBinding<UniformBuffers>(0, 0, VK_SHADER_STAGE_VERTEX_BIT);
    mDescriptorManager->createDescriptorResources(mSwapChain->getSwapChainFramebuffers().size());

    mBaseShader = new BaseShader();
    createPipeline();
    createCommandBuffers();
    createSyncObjects();
}

void Application::createGeometry() {
    positions.clear();
    colors.clear();
    indices.clear();

    // 创建网格顶点 - 在XZ平面上
    for (int i = -GRID_SIZE; i <= GRID_SIZE; i += GRID_STEP) {
        // X方向线
        positions.push_back(glm::vec3(i, 0.0f, -GRID_SIZE));
        positions.push_back(glm::vec3(i, 0.0f, GRID_SIZE));

        // Z方向线
        positions.push_back(glm::vec3(-GRID_SIZE, 0.0f, i));
        positions.push_back(glm::vec3(GRID_SIZE, 0.0f, i));

        // 设置颜色 - 中心轴线更亮
        if (i == 0) {
            // X轴 - 红色
            colors.push_back(glm::vec3(1.0f, 0.2f, 0.2f));
            colors.push_back(glm::vec3(1.0f, 0.2f, 0.2f));

            // Z轴 - 蓝色
            colors.push_back(glm::vec3(0.4f, 0.4f, 1.0f));
            colors.push_back(glm::vec3(0.4f, 0.4f, 1.0f));
        }
        else {
            // 普通网格线 - 灰色
            float intensity = abs(i) % 10 == 0 ? 0.6f : 0.3f;
            colors.push_back(glm::vec3(intensity));
            colors.push_back(glm::vec3(intensity));
            colors.push_back(glm::vec3(intensity));
            colors.push_back(glm::vec3(intensity));
        }
    }

    // 创建索引
    for (uint32_t i = 0; i < positions.size(); i++) {
        indices.push_back(i);
    }
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

    delete mDescriptorManager;
    mDescriptorManager = nullptr;
    mIndexBuffer = nullptr;
    delete mBaseShader;
    mBaseShader = nullptr;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        mImageAvailableSemaphores[i].reset();
        mRenderFinishedSemaphores[i].reset();
        mInFlightFences[i].reset();
    }

    mCommandPool.reset();
    mLogicalDevice.reset();
    mPhysicalDevice.reset();
    mDebug.reset();
    mInstance.reset();
    mWindow.reset();
}

void Application::createRenderPass() {
    mRenderPass = RenderPass::create(mLogicalDevice->getHandle());
    // 添加颜色附件
    mRenderPass->addAttachment({
        RenderPass::Type::Color,
        mSwapChain->getSwapChainImageFormat(),
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        })
        .addAttachment({
            RenderPass::Type::Depth,
            Texture::findSupportedDepthFormat(mPhysicalDevice->getHandle()),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            })
        .addDependency({
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            })
        .build();
}

void Application::createPipeline() {
    mPipeline = Pipeline::create(mLogicalDevice);

    mPipeline->setRenderPass(mRenderPass);

    auto vertexInput = VertexInput();
    auto bindings = mVertexBuffer->getBindingDescriptions();
    auto attributes = mVertexBuffer->getAttributeDescriptions();
    for (const auto& binding : bindings) {
        vertexInput.addBinding(binding.binding, binding.stride);
    }
    for (const auto& attr : attributes) {
        vertexInput.addAttribute(attr.binding, attr);
    }
    mPipeline->setVertexInputState(vertexInput);

    auto inputAssembly= InputAssembly();
    inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

    mPipeline->setInputAssemblyState(inputAssembly);

    // 设置光栅化状态为线框模式
    auto rasterization = Rasterization();
    rasterization.setPolygonMode(VK_POLYGON_MODE_LINE) // 线框模式
        .setLineWidth(1.0f)                  // 线宽
        .setCullMode(VK_CULL_MODE_NONE);     // 禁用背面剔除
    mPipeline->setRasterizationState(rasterization);

    // 禁用深度测试
    auto depthStencil = DepthStencil();
    depthStencil.enableDepthTest(VK_FALSE)
        .enableDepthWrite(VK_FALSE);
    mPipeline->setDepthStencilState(depthStencil);

    mPipeline->setViewportState(Viewport().IsOpenglCoordinate().createViewport(mSwapChain->getSwapChainExtent()));

    const auto descriptorLayouts = mDescriptorManager->getDescriptorSetLayouts();
    mPipeline->setPipelineLayout(PipelineLayout::create(mLogicalDevice, descriptorLayouts));
    //mPipeline->setPipelineLayout(PipelineLayout::create(mLogicalDevice, {}));

    mPipeline->setShaderStage(mBaseShader->buildDefaultShaderStages(mLogicalDevice));

    mPipeline->createGraphicsPipeline();
}

void Application::createCommandBuffers() {
    auto framebuffers = mSwapChain->getSwapChainFramebuffers();
    auto extent = mSwapChain->getSwapChainExtent();
    mCommandBuffers.resize(framebuffers.size());

    for (size_t i = 0; i < framebuffers.size(); ++i) {
        mCommandBuffers[i] = CommandBuffer::create(mLogicalDevice, mCommandPool);
        mCommandBuffers[i]->begin();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.3f, 0.3f, 0.3f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

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

        mCommandBuffers[i]->bindDescriptorSets(mPipeline->getPipelineLayout()->getHandle(),mDescriptorManager->getDescriptorSetsForFrame(i));
        mCommandBuffers[i]->bindVertexBuffers(mVertexBuffer->getBufferHandles());
        mCommandBuffers[i]->bindIndexBuffer(mIndexBuffer->getBuffer());
        mCommandBuffers[i]->drawIndexed(static_cast<uint32_t>(indices.size()));
        //mCommandBuffers[i]->draw(3);

        mCommandBuffers[i]->endRenderPass();
        mCommandBuffers[i]->end();
    }

}

void Application::recreateSwapChain() {
    vkDeviceWaitIdle(mLogicalDevice->getHandle());
    cleanupSwapChain();
    mSwapChain = SwapChain::create(mLogicalDevice, mWindow);
    createRenderPass();
    mSwapChain->createFramebuffers(mRenderPass->getHandle(), mCommandPool);
    mDescriptorManager->createDescriptorResources(mSwapChain->getSwapChainFramebuffers().size());
    createPipeline();
    createCommandBuffers();
}
void Application::cleanupSwapChain() {
    mCommandBuffers.clear();
    mPipeline.reset();

    if (mDescriptorManager) {
        mDescriptorManager->cleanupResources();
    }

    mSwapChain->cleanupFramebuffers();
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
        mInFlightFences[i] = Fence::create(mLogicalDevice);
    }
}

void Application::drawFrame() {
    FrameData frameData = acquireNextImage();

    if (frameData.acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (frameData.acquireResult != VK_SUCCESS && frameData.acquireResult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    mInFlightFences[mCurrentFrame]->block();

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBuffers ubo{};

    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    ubo.view = glm::lookAt(
        glm::vec3(0.0f, 10.0f, 10.0f),  // 相机抬高到(0,10,10)
        glm::vec3(0.0f, 0.0f, 0.0f),    // 看向网格中心
        glm::vec3(0.0f, 1.0f, 0.0f));   // 使用标准Y轴向上

    float aspect = mSwapChain->getSwapChainExtent().width /
        (float)mSwapChain->getSwapChainExtent().height;

    ubo.proj = glm::perspective(
        glm::radians(60.0f),            
        aspect,
        0.1f,                           
        100.0f);

    /*ubo.proj[1][1] *= -1;*/ 

    //UniformBufferObject ubo{};
    //float aspect = static_cast<float>(mSwapChain->getSwapChainExtent().width) /
    //    static_cast<float>(mSwapChain->getSwapChainExtent().height);

    //// 考虑宽高比的正交投影
    //ubo.mvp = glm::ortho(
    //    -halfWidth, halfWidth,        // left, right
    //    -halfHeight / aspect, halfHeight / aspect, // bottom, top (调整高度)
    //    -1.0f, 1.0f                   // near, far
    //);
    mDescriptorManager->updateUniformData<UniformBuffers>(0, 0, frameData.imageIndex, ubo);

    //mDescriptorManager->updateUniformData<UniformBuffers>(0, 0, frameData.imageIndex, ubo);

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

