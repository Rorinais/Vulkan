// src/rendering/config/InputAssemblyConfig.cpp
#include "InputAssemblyConfig.hpp"

InputAssemblyConfig& InputAssemblyConfig::setTopology(VkPrimitiveTopology topology) {
    m_config.topology = topology;
    return *this;
}

InputAssemblyConfig& InputAssemblyConfig::enablePrimitiveRestart(VkBool32 enable) {
    m_config.primitiveRestartEnable = enable;
    return *this;
}

const VkPipelineInputAssemblyStateCreateInfo& InputAssemblyConfig::getCreateInfo() const {
    static VkPipelineInputAssemblyStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = m_config.topology;
    info.primitiveRestartEnable = m_config.primitiveRestartEnable;
    return info;
}