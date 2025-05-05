// VertexInputConfig.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VertexInputConfig {
public:
    VertexInputConfig& setBindings(const std::vector<VkVertexInputBindingDescription>& bindings);

    VertexInputConfig& setAttributes(const std::vector<VkVertexInputAttributeDescription>& attributes);

    VertexInputConfig& addBinding(uint32_t binding, uint32_t stride);

    VertexInputConfig& addAttribute(uint32_t binding, const VkVertexInputAttributeDescription& attr);

    const VkPipelineVertexInputStateCreateInfo& getCreateInfo();

private:
    std::vector<VkVertexInputBindingDescription> m_bindings;
    std::vector<VkVertexInputAttributeDescription> m_attributes;
    VkPipelineVertexInputStateCreateInfo m_createInfo{};
};