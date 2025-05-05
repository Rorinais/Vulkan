#include "VertexInputConfig.hpp"

VertexInputConfig& VertexInputConfig::setBindings(const std::vector<VkVertexInputBindingDescription>& bindings) {
    m_bindings = bindings;
    return *this;
}

VertexInputConfig& VertexInputConfig::setAttributes(const std::vector<VkVertexInputAttributeDescription>& attributes) {
    m_attributes = attributes;
    return *this;
}

// 保留原有方法以兼容
VertexInputConfig& VertexInputConfig::addBinding(uint32_t binding, uint32_t stride) {
    VkVertexInputBindingDescription desc{};
    desc.binding = binding;
    desc.stride = stride;
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_bindings.push_back(desc);
    return *this;
}

VertexInputConfig& VertexInputConfig::addAttribute(uint32_t binding, const VkVertexInputAttributeDescription& attr) {
    m_attributes.push_back(attr);
    return *this;
}

const VkPipelineVertexInputStateCreateInfo& VertexInputConfig::getCreateInfo() {
    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindings.size());
    m_createInfo.pVertexBindingDescriptions = m_bindings.data();
    m_createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributes.size());
    m_createInfo.pVertexAttributeDescriptions = m_attributes.data();
    return m_createInfo;
}