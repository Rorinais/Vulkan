#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VertexInputConfig {
public:
    struct Config {
        uint32_t location;
        VkFormat format;
        uint32_t offset;
    };

    VertexInputConfig& addBinding(uint32_t binding, uint32_t stride);
    VertexInputConfig& addAttribute(uint32_t binding, Config attr);

    const VkPipelineVertexInputStateCreateInfo& getCreateInfo();

private:
    std::vector<VkVertexInputBindingDescription> m_bindings;
    std::vector<VkVertexInputAttributeDescription> m_attributes;
    VkPipelineVertexInputStateCreateInfo m_createInfo{};
};