#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class ColorBlendConfig {
public:
    struct AttachmentConfig {
        VkBool32 blendEnable = VK_FALSE;
        VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
        VkColorComponentFlags colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    };

    ColorBlendConfig& addAttachment(const AttachmentConfig& config);

    const VkPipelineColorBlendStateCreateInfo& getCreateInfo() const;

private:
    std::vector<VkPipelineColorBlendAttachmentState> m_attachments;
    VkPipelineColorBlendStateCreateInfo m_createInfo{};
};