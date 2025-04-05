// include/rendering/config/InputAssemblyConfig.hpp
#pragma once
#include <vulkan/vulkan.h>

class InputAssemblyConfig {
public:
    struct Config {
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
    };

    InputAssemblyConfig& setTopology(VkPrimitiveTopology topology);
    InputAssemblyConfig& enablePrimitiveRestart(VkBool32 enable);

    const VkPipelineInputAssemblyStateCreateInfo& getCreateInfo() const;

private:
    Config m_config;
    VkPipelineInputAssemblyStateCreateInfo m_createInfo{};
};