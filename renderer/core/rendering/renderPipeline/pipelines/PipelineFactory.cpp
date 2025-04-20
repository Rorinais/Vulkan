#include "PipelineFactory.hpp"

PipelineFactory::PipelineFactory(VkDevice device): mDevice(device) {}
PipelineFactory::~PipelineFactory() {
    cleanup();
}

void PipelineFactory::configure(const PipelineConfig& config) {
    mConfig = config;
}

void PipelineFactory::createRenderPass(RenderPassBuilder::RenderPassConfig&config) {
    if (mRenderPass == VK_NULL_HANDLE) { 
        RenderPassBuilder passbuilder(mDevice);
        mRenderPass = passbuilder.configureColorAttachment(config).build();
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