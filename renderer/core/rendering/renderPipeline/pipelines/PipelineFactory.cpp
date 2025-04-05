#include "PipelineFactory.hpp"

PipelineFactory::PipelineFactory(VkDevice device): m_device(device) {}

void PipelineFactory::configure(const PipelineConfig& config) {
    m_config = config;
}

VkPipeline PipelineFactory::createGraphicsPipeline(
    VkRenderPass renderPass,
    VkExtent2D swapChainExtent,
    VkShaderModule vertModule,
    VkShaderModule fragModule){

    // 2. ��ɫ���׶�����
    VkPipelineShaderStageCreateInfo stages[2] = {
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", nullptr},
        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
         VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", nullptr}
    };

    // 3. ������������
    VertexInputConfig vertexInput;

    // 4. ����װ������
    InputAssemblyConfig inputAssembly;

    // 5. �ӿ�����
    ViewportConfig viewport(swapChainExtent);

    // 6.��դ������
    RasterizationConfig rasterization;

    // 7.���ز�������
    MultisampleConfig Multisample;

    // 8.��ɫ�������
    ColorBlendConfig colorBlend;
    colorBlend.addAttachment({
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT |
                      VK_COLOR_COMPONENT_A_BIT
        });

    // 9. ��������
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = &vertexInput.getCreateInfo();
    createInfo.pInputAssemblyState = &inputAssembly.getCreateInfo();
    createInfo.pViewportState = &viewport.getCreateInfo();
    createInfo.pRasterizationState = &rasterization.getCreateInfo();
    createInfo.pMultisampleState = &Multisample.getCreateInfo();
    createInfo.pColorBlendState = &colorBlend.getCreateInfo();
    createInfo.layout = mPipelineLayout;
    createInfo.renderPass = renderPass;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    // 10. ������ʱ��Դ
    vkDestroyShaderModule(m_device, vertModule, nullptr);
    vkDestroyShaderModule(m_device, fragModule, nullptr);
    return pipeline;
}