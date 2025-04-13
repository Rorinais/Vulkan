#include "PipelineFactory.hpp"

PipelineFactory::PipelineFactory(VkDevice device): mDevice(device) {}
PipelineFactory::~PipelineFactory() {
    cleanup();
}

void PipelineFactory::configure(const PipelineConfig& config) {
    mConfig = config;
}

void PipelineFactory::createRenderPass(VkFormat swapChainImageFormat) {
    if (mRenderPass == VK_NULL_HANDLE) { 
        RenderPassBuilder::RenderPassConfig config{
            .colorFormat = swapChainImageFormat,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
        };
        RenderPassBuilder passbuilder(mDevice);
        mRenderPass = passbuilder.configureColorAttachment(config).build();
    }
}


void PipelineFactory::createGraphicsPipeline(
    VkShaderModule vertModule,
    VkShaderModule fragModule){
    VkPipelineShaderStageCreateInfo stages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", nullptr}
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

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
    createInfo.layout = mPipelineLayout;
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
        if (mPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
            mPipelineLayout = VK_NULL_HANDLE;
        }
        if (mRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
            mRenderPass = VK_NULL_HANDLE;
        }
    }
}