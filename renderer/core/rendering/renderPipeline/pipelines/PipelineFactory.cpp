#include "PipelineFactory.hpp"

PipelineFactory::PipelineFactory(VkDevice device): mDevice(device) {}
PipelineFactory::~PipelineFactory() {
    cleanup();
}

void PipelineFactory::configure(const PipelineConfig& config) {
    mConfig = config;

    // 深度模板状态配置
    mConfig.depthStencil = {}; // 重要：清空原有数据
    mConfig.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    mConfig.depthStencil.pNext = nullptr; // 明确设置pNext为nullptr
    mConfig.depthStencil.flags = 0;       // 显式设置flags为0

    // 深度测试配置
    mConfig.depthStencil.depthTestEnable = VK_TRUE;
    mConfig.depthStencil.depthWriteEnable = VK_TRUE;
    mConfig.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    mConfig.depthStencil.depthBoundsTestEnable = VK_FALSE;
    mConfig.depthStencil.minDepthBounds = 0.0f;
    mConfig.depthStencil.maxDepthBounds = 1.0f;

    // 模板测试配置（即使未启用也需初始化）
    mConfig.depthStencil.stencilTestEnable = VK_FALSE;
    // 初始化front模板操作
    mConfig.depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
    mConfig.depthStencil.front.passOp = VK_STENCIL_OP_KEEP;
    mConfig.depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
    mConfig.depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
    mConfig.depthStencil.front.compareMask = 0;
    mConfig.depthStencil.front.writeMask = 0;
    mConfig.depthStencil.front.reference = 0;
    // 复制front配置到back
    mConfig.depthStencil.back = mConfig.depthStencil.front;
}

void PipelineFactory::createRenderPass(RenderPassBuilder::RenderPassConfig&config, VkFormat& depthFormat) {
    if (mRenderPass == VK_NULL_HANDLE) { 
        RenderPassBuilder passbuilder(mDevice);
        mRenderPass = passbuilder.configureColorAttachment(config).configureDepthAttachment(depthFormat).build();
    }
}


void PipelineFactory::createGraphicsPipeline(VkShaderModule vertModule,VkShaderModule fragModule){
    VkPipelineShaderStageCreateInfo stages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", nullptr}
    };

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = &mConfig.vertexInput.getCreateInfo();
    createInfo.pInputAssemblyState = &mConfig.inputAssembly.getCreateInfo();
    createInfo.pViewportState = &mConfig.viewport.getCreateInfo();
    createInfo.pRasterizationState = &mConfig.rasterization.getCreateInfo();
    createInfo.pMultisampleState = &mConfig.multisample.getCreateInfo();
    createInfo.pColorBlendState = &mConfig.colorBlend.getCreateInfo();
    createInfo.pDepthStencilState = &mConfig.depthStencil;
    createInfo.layout = mConfig.pipelineLayout;
    createInfo.renderPass = mRenderPass;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

#if DYNAMIC_STATE
    std::vector<VkDynamicState> dynamicStates = {
       VK_DYNAMIC_STATE_VIEWPORT,
       VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    createInfo.pDynamicState = &dynamicState;
#endif 

    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    vkDestroyShaderModule(mDevice, vertModule, nullptr);
    vkDestroyShaderModule(mDevice, fragModule, nullptr);
}

void PipelineFactory::cleanup() {
    if (mDevice != VK_NULL_HANDLE) {
        if (mGraphicsPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
            mGraphicsPipeline = VK_NULL_HANDLE;
        }
        if (mConfig.pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice, mConfig.pipelineLayout, nullptr);
            mConfig.pipelineLayout = VK_NULL_HANDLE;
        }
        if (mRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
            mRenderPass = VK_NULL_HANDLE;
        }
    }
}