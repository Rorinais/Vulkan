#include"pipeline.hpp"

Pipeline::Pipeline(const LogicalDevice::Ptr& logicalDevice, Pipeline::Config config)
    :mLogicalDevice(logicalDevice),mPipelineStateConfig(config) {

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
    setColorBlendState(colorBlend);

    auto depthStencil = DepthStencil();
    // depthStencil.enableDepthTest();  
    // depthStencil.enableDepthWrite();
    // depthStencil.setDepthCompareOp(VK_COMPARE_OP_LESS);
    setDepthStencilState(depthStencil, false);  

    auto inputAssembly = InputAssembly();
    inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); 
    inputAssembly.enablePrimitiveRestart(VK_FALSE); 
    setInputAssemblyState(inputAssembly);

    auto multiSample = MultiSample();
    multiSample.enableSampleShading(VK_FALSE);  
    multiSample.setSampleCount(VK_SAMPLE_COUNT_1_BIT);  
    setMultiSampleState(multiSample);

    auto rasterization = Rasterization();
    rasterization.setCullMode(VK_CULL_MODE_NONE)
        .setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    setRasterizationState(rasterization);

    auto dynamic = Dynamic();
    dynamic.setEnableDynamic(true);
    setDynamicStates(dynamic);

    const std::vector<VkDescriptorSetLayout> descriptorSetLayout;
    auto pipelineLayout = PipelineLayout::create(mLogicalDevice, descriptorSetLayout);
    setPipelineLayout(pipelineLayout);

    setSubPass(0);

}

Pipeline::~Pipeline() {
	cleanup();
}

void Pipeline::cleanup() {
    if (mLogicalDevice->getHandle() != VK_NULL_HANDLE) {
        if (mGraphicsPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mLogicalDevice->getHandle(), mGraphicsPipeline, nullptr);
            mGraphicsPipeline = VK_NULL_HANDLE;
        }
    }
}

void Pipeline::setShaderStage(ShaderStages::Ptr shaderStates) {
    mShaderStages = shaderStates;
}
void Pipeline::setVertexInputState(VertexInput vertexInput) {
    mPipelineStateConfig.vertexInputState = vertexInput;
}
void Pipeline::setInputAssemblyState(InputAssembly inputAssembly) {
    mPipelineStateConfig.inputAssemblyState = inputAssembly;
}
void Pipeline::setViewportState(Viewport viewport) {
    mPipelineStateConfig.viewportState = viewport;
}
void Pipeline::setRasterizationState(Rasterization rasterization) {
    mPipelineStateConfig.rasterizationState = rasterization;
}
void Pipeline::setMultiSampleState(MultiSample multiSample) {
    mPipelineStateConfig.multisampleState = multiSample;
}
void Pipeline::setColorBlendState(ColorBlend colorBlend) {
    mPipelineStateConfig.colorBlendState = colorBlend;
}
void Pipeline::setDepthStencilState(DepthStencil depthStencil, bool enable) {
    mEnableDepth = enable;
    mPipelineStateConfig.depthStencilState = depthStencil;
}

void Pipeline::setDynamicStates(Dynamic dynamic) {
    mPipelineStateConfig.dynamicState = dynamic;
}

void Pipeline::setPipelineLayout(PipelineLayout::Ptr pipelineLayout) {
    mPipelineLayout = pipelineLayout;
}
void Pipeline::setRenderPass(RenderPass::Ptr renderPass) {
    mRenderPass = renderPass;
}
void Pipeline::setSubPass(uint32_t count) {
    mSubpass = count;
}
void Pipeline::setBasePipelineHandle(VkPipeline basePipeline) {
    mBasePipelineHandle = basePipeline;
}
void Pipeline::setBasePipelineIndex(uint32_t index) {
    mBasePipelineIndex = index;
}

void Pipeline::createGraphicsPipeline() {
    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = static_cast<uint32_t>(mShaderStages->getStages().size());
    createInfo.pStages = mShaderStages->getStages().data();
    createInfo.pVertexInputState = &mPipelineStateConfig.vertexInputState.getCreateInfo();
    createInfo.pInputAssemblyState = &mPipelineStateConfig.inputAssemblyState.getCreateInfo();
    createInfo.pViewportState = &mPipelineStateConfig.viewportState.getCreateInfo();
    createInfo.pRasterizationState = &mPipelineStateConfig.rasterizationState.getCreateInfo();
    createInfo.pMultisampleState = &mPipelineStateConfig.multisampleState.getCreateInfo();
    createInfo.pColorBlendState = &mPipelineStateConfig.colorBlendState.getCreateInfo();
    createInfo.pDynamicState = &mPipelineStateConfig.dynamicState.getCreateInfo();
    createInfo.layout = mPipelineLayout->getHandle();
    createInfo.renderPass = mRenderPass->getHandle();
    createInfo.subpass = mSubpass;
    createInfo.basePipelineHandle = mBasePipelineHandle;
    createInfo.basePipelineIndex = mBasePipelineIndex;
    if (mEnableDepth) {
        createInfo.pDepthStencilState = &mPipelineStateConfig.depthStencilState.getCreateInfo();
    }
    else {
        createInfo.pDepthStencilState = nullptr;
    }

    if (vkCreateGraphicsPipelines(mLogicalDevice->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
}