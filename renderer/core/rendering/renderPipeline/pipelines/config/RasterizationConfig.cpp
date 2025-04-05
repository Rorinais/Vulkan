// src/rendering/config/RasterizationConfig.cpp
#include "RasterizationConfig.hpp"

RasterizationConfig& RasterizationConfig::setPolygonMode(VkPolygonMode mode) {
    m_config.polygonMode = mode;
    return *this;
}

RasterizationConfig& RasterizationConfig::setCullMode(VkCullModeFlags cullMode) {
    m_config.cullMode = cullMode;
    return *this;
}

RasterizationConfig& RasterizationConfig::setFrontFace(VkFrontFace frontFace) {
    m_config.frontFace = frontFace;
    return *this;
}

const VkPipelineRasterizationStateCreateInfo& RasterizationConfig::getCreateInfo() const {
    static VkPipelineRasterizationStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.polygonMode = m_config.polygonMode;
    info.cullMode = m_config.cullMode;
    info.frontFace = m_config.frontFace;
    info.lineWidth = m_config.lineWidth;

    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.depthBiasEnable = VK_FALSE;
    return info;
}