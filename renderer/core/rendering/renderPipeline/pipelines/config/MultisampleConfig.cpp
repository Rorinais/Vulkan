// src/rendering/config/MultisampleConfig.cpp
#include "MultisampleConfig.hpp"

MultisampleConfig& MultisampleConfig::setSampleCount(VkSampleCountFlagBits count) {
    m_config.rasterizationSamples = count;
    return *this;
}

MultisampleConfig& MultisampleConfig::enableSampleShading(VkBool32 enable) {
    m_config.sampleShadingEnable = enable;
    return *this;
}

const VkPipelineMultisampleStateCreateInfo& MultisampleConfig::getCreateInfo() const {
    static VkPipelineMultisampleStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.rasterizationSamples = m_config.rasterizationSamples;
    info.sampleShadingEnable = m_config.sampleShadingEnable;
    return info;
}