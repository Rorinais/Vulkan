#include "VulkanRenderPass.hpp"

VulkanRenderPass::VulkanRenderPass(Type type) : RenderPass(type) {}

VulkanRenderPass::~VulkanRenderPass() {
    destroy();
}

VulkanRenderPass& VulkanRenderPass::addAttachment(const AttachmentConfig& config) {
    mAttachmentConfigs.push_back(config);
    return *this;
}

VulkanRenderPass& VulkanRenderPass::addDependency(const DependencyConfig& config) {
    mDependencyConfigs.push_back(config);
    return *this;
}

void VulkanRenderPass::build() {
    // 创建VkRenderPass
    std::vector<VkAttachmentDescription> attachments;
    for (const auto& config : mAttachmentConfigs) {
        VkAttachmentDescription attachment{};
        attachment.format = config.format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = config.loadOp;
        attachment.storeOp = config.storeOp;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = config.initialLayout;
        attachment.finalLayout = config.finalLayout;
        attachments.push_back(attachment);
    }

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkSubpassDependency> dependencies;
    for (const auto& config : mDependencyConfigs) {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = config.srcSubpass;
        dependency.dstSubpass = config.dstSubpass;
        dependency.srcStageMask = config.srcStageMask;
        dependency.dstStageMask = config.dstStageMask;
        dependency.srcAccessMask = config.srcAccessMask;
        dependency.dstAccessMask = config.dstAccessMask;
        dependencies.push_back(dependency);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(mVulkanCore->getLogicalDeviceHandle(), &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanRenderPass::init(VulkanCore::Ptr vulkanCore, WindowContext::Ptr windowContext) {
    mVulkanCore = vulkanCore;
    mWindowContext = windowContext;

    build();
}

void VulkanRenderPass::destroy() {
    if (!mVulkanCore || !mVulkanCore->getLogicalDeviceHandle()) return;

    vkDeviceWaitIdle(mVulkanCore->getLogicalDeviceHandle());

    for (auto framebuffer : mSwapchainFramebuffers) {
        vkDestroyFramebuffer(mVulkanCore->getLogicalDeviceHandle(), framebuffer, nullptr);
    }
    mSwapchainFramebuffers.clear();

    if (mRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(mVulkanCore->getLogicalDeviceHandle(), mRenderPass, nullptr);
        mRenderPass = VK_NULL_HANDLE;
    }

    mPipeline.reset();
}

void VulkanRenderPass::createPipeline() {
    Pipeline::Config config;

    auto colorBlend = ColorBlend();
    ColorBlend::Config colorBlendConfig{};
    colorBlendConfig.blendEnable = VK_FALSE;
    colorBlendConfig.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendConfig.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendConfig.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendConfig.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendConfig.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendConfig.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendConfig.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    colorBlend.addAttachment(colorBlendConfig);
    config.colorBlendState = colorBlend;

    auto depthStencil = DepthStencil();
    depthStencil.enableDepthTest(VK_TRUE);  // 启用深度测试
    depthStencil.enableDepthWrite(VK_TRUE); // 启用深度写入
    depthStencil.setDepthCompareOp(VK_COMPARE_OP_LESS);
    config.depthStencilState = depthStencil;

    auto inputAssembly = InputAssembly();
    inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    inputAssembly.enablePrimitiveRestart(VK_FALSE);
    config.inputAssemblyState = inputAssembly;

    auto multiSample = MultiSample();
    multiSample.enableSampleShading(VK_FALSE);
    multiSample.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
    config.multisampleState = multiSample;

    auto rasterization = Rasterization();
    rasterization.setCullMode(VK_CULL_MODE_BACK_BIT) // 启用背面剔除
        .setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
        .setLineWidth(1.0f);
    config.rasterizationState = rasterization;

    auto dynamic = Dynamic();
    dynamic.setEnableDynamic(true);

    config.dynamicState = dynamic;

    // 创建管道
    mPipeline = Pipeline::create(mVulkanCore->getLogicalDevice(), config);

    // 传递当前 RenderPass
    mPipeline->setRenderPass(this);

    // 配置顶点输入
    auto vertexInput = VertexInput();

    auto bindings = mMesh.getVertexBuffer()->getBindingDescriptions();
    auto attributes = mMesh.getVertexBuffer()->getAttributeDescriptions();

    for (const auto& binding : bindings) {
        vertexInput.addBinding(binding.binding, binding.stride);
    }
    for (const auto& attr : attributes) {
        vertexInput.addAttribute(attr.binding, attr);
    }

    mPipeline->setVertexInputState(vertexInput);

    // 配置视口
    mPipeline->setViewportState(Viewport().IsOpenglCoordinate().createViewport(mWindowContext->getSwapchainExtent()));

    if (mDescriptorSetLayouts.empty()) {
        throw std::runtime_error("No descriptor set layouts provided for pipeline!");
    }

    std::cout << "Creating pipeline with "
        << mDescriptorSetLayouts.size()
        << " descriptor set layouts\n";

    // 创建管道布局
    mPipeline->setPipelineLayout(PipelineLayout::create(mVulkanCore->getLogicalDevice(), mDescriptorSetLayouts));

    // 设置着色器
    if (mShaderProgram) {
        mPipeline->setShaderStage(mShaderProgram);
    }

    // 创建图形管道
    mPipeline->createGraphicsPipeline();
}

void VulkanRenderPass::buildPipeline() {
    Pipeline::Config config;

    auto colorBlend = ColorBlend();
    ColorBlend::Config colorBlendConfig{};
    colorBlendConfig.blendEnable = VK_FALSE;
    colorBlendConfig.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendConfig.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendConfig.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendConfig.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendConfig.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendConfig.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendConfig.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    colorBlend.addAttachment(colorBlendConfig);
    config.colorBlendState = colorBlend;

    auto depthStencil = DepthStencil();
    depthStencil.enableDepthTest(VK_TRUE);  // 启用深度测试
    depthStencil.enableDepthWrite(VK_TRUE); // 启用深度写入
    depthStencil.setDepthCompareOp(VK_COMPARE_OP_LESS);
    config.depthStencilState = depthStencil;

    auto inputAssembly = InputAssembly();
    inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    inputAssembly.enablePrimitiveRestart(VK_FALSE);
    config.inputAssemblyState = inputAssembly;

    auto multiSample = MultiSample();
    multiSample.enableSampleShading(VK_FALSE);
    multiSample.setSampleCount(VK_SAMPLE_COUNT_1_BIT);
    config.multisampleState = multiSample;

    auto rasterization = Rasterization();
    rasterization.setCullMode(VK_CULL_MODE_BACK_BIT) // 启用背面剔除
        .setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
        .setLineWidth(1.0f);
    config.rasterizationState = rasterization;

    auto dynamic = Dynamic();
    dynamic.setEnableDynamic(true);

    config.dynamicState = dynamic;

    // 创建管道
    mPipeline = Pipeline::create(mVulkanCore->getLogicalDevice(), config);

    // 传递当前 RenderPass
    mPipeline->setRenderPass(this);

    // 配置顶点输入
    auto vertexInput = VertexInput();

    auto bindings = mMesh.getVertexBuffer()->getBindingDescriptions();
    auto attributes = mMesh.getVertexBuffer()->getAttributeDescriptions();

    for (const auto& binding : bindings) {
        vertexInput.addBinding(binding.binding, binding.stride);
    }
    for (const auto& attr : attributes) {
        vertexInput.addAttribute(attr.binding, attr);
    }

    mPipeline->setVertexInputState(vertexInput);

    // 配置视口
    mPipeline->setViewportState(Viewport().IsOpenglCoordinate().createViewport(mWindowContext->getSwapchainExtent()));

    if (mDescriptorSetLayouts.empty()) {
        throw std::runtime_error("No descriptor set layouts provided for pipeline!");
    }

    std::cout << "Creating pipeline with "
        << mDescriptorSetLayouts.size()
        << " descriptor set layouts\n";

    // 创建管道布局
    mPipeline->setPipelineLayout(PipelineLayout::create(mVulkanCore->getLogicalDevice(), mDescriptorSetLayouts));

    // 设置着色器
    if (mShaderProgram) {
        mPipeline->setShaderStage(mShaderProgram);
    }

    // 创建图形管道
    mPipeline->createGraphicsPipeline();
}

void VulkanRenderPass::createFramebuffers() {
    if (!mResourceManager) {
        throw std::runtime_error("ResourceManager not set in VulkanRenderPass");
    }

    // 通过资源管理器获取交换链资源
    auto swapchainImageViews = mResourceManager->getSwapchainImageViews();
    auto depthTexture = mResourceManager->getDepthTexture();

    // 清理旧的帧缓冲（如果有）
    for (auto framebuffer : mSwapchainFramebuffers) {
        vkDestroyFramebuffer(mVulkanCore->getLogicalDeviceHandle(), framebuffer, nullptr);
    }
    mSwapchainFramebuffers.clear();

    mSwapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::vector<VkImageView> attachments = {
            swapchainImageViews[i],
            depthTexture->getImageView()
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = mWindowContext->getSwapchainExtent().width;
        framebufferInfo.height = mWindowContext->getSwapchainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mVulkanCore->getLogicalDeviceHandle(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void VulkanRenderPass::beginFrame(uint32_t imageIndex) {
    // 清屏值
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mWindowContext->getSwapchainExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
}

void VulkanRenderPass::recordCommands(CommandBuffer::Ptr cmdBuffer, uint32_t imageIndex, uint32_t frameIndex) {
    // 确保命令缓冲区处于记录状态
    if (!cmdBuffer->isRecording()) {
        throw std::runtime_error("Command buffer not in recording state");
    }

    // 清屏值
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mWindowContext->getSwapchainExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    // 直接在传入的命令缓冲区上记录
    cmdBuffer->beginRenderPass(renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    cmdBuffer->setViewport(mWindowContext->getSwapchainExtent());
    cmdBuffer->setScissor(mWindowContext->getSwapchainExtent());
    cmdBuffer->bindGraphicPipeline(mPipeline->getHandle());

    if (!mDescriptorSets.empty() && frameIndex < mDescriptorSets.size()) {
        cmdBuffer->bindDescriptorSets(mPipeline->getPipelineLayout()->getHandle(),0,  mDescriptorSets[frameIndex]);
    }

    // 绑定顶点和索引缓冲区
    cmdBuffer->bindVertexBuffers(mMesh.getVertexBuffer()->getBufferHandles());
    cmdBuffer->bindIndexBuffer(mMesh.getIndexBuffer()->getBuffer());

    // 绘制命令
    cmdBuffer->drawIndexed(mMesh.getIndexBuffer()->getIndexCount());

    cmdBuffer->endRenderPass();
}

void VulkanRenderPass::endFrame() {
    // 结束帧处理（如有需要）
}

void VulkanRenderPass::onSwapChainRecreated() {
    // 销毁旧的RenderPass和帧缓冲
    if (mRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(mVulkanCore->getLogicalDeviceHandle(), mRenderPass, nullptr);
        mRenderPass = VK_NULL_HANDLE;
    }
    for (auto fb : mSwapchainFramebuffers) {
        vkDestroyFramebuffer(mVulkanCore->getLogicalDeviceHandle(), fb, nullptr);
    }
    mSwapchainFramebuffers.clear();

    // 重建RenderPass和帧缓冲
    build();
    createFramebuffers();
    createPipeline();
}