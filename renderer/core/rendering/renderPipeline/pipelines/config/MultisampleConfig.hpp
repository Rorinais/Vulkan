#pragma once
#include <vulkan/vulkan.h>

class MultisampleConfig {
public:
    struct Config {
        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 sampleShadingEnable = VK_FALSE;
    };

    MultisampleConfig& setSampleCount(VkSampleCountFlagBits count);
    MultisampleConfig& enableSampleShading(VkBool32 enable);

    const VkPipelineMultisampleStateCreateInfo& getCreateInfo() const;

private:
    Config m_config;
    VkPipelineMultisampleStateCreateInfo m_createInfo{};
};