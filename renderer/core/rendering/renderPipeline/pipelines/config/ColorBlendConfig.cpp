// src/rendering/config/ColorBlendConfig.cpp
#include "ColorBlendConfig.hpp"

ColorBlendConfig& ColorBlendConfig::addAttachment(const AttachmentConfig& config){
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.blendEnable = config.blendEnable;
    attachment.srcColorBlendFactor = config.srcColorBlendFactor;
    attachment.dstColorBlendFactor = config.dstColorBlendFactor;
    attachment.colorBlendOp = config.colorBlendOp;
    attachment.colorWriteMask = config.colorWriteMask;
    m_attachments.push_back(attachment);
    return *this;
}

const VkPipelineColorBlendStateCreateInfo& ColorBlendConfig::getCreateInfo() const {
    static VkPipelineColorBlendStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_COPY;
    info.attachmentCount = static_cast<uint32_t>(m_attachments.size());
    info.pAttachments = m_attachments.data();
    info.blendConstants[0] = 0.0f;
    info.blendConstants[1] = 0.0f;
    info.blendConstants[2] = 0.0f;
    info.blendConstants[3] = 0.0f;
    return info;
}