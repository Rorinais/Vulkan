#pragma once
#include <vulkan/vulkan.h>

class DepthStencilConfig {
public:
    VkPipelineDepthStencilStateCreateInfo getCreateInfo() const {
        VkPipelineDepthStencilStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        createInfo.depthTestEnable = VK_TRUE;
        createInfo.depthWriteEnable = VK_TRUE;
        createInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        createInfo.depthBoundsTestEnable = VK_FALSE;
        createInfo.minDepthBounds = 0.0f;
        createInfo.maxDepthBounds = 1.0f;
        createInfo.stencilTestEnable = VK_FALSE;
        return createInfo;
    }
};