// include/rendering/config/ViewportConfig.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class ViewportConfig {
public:
    struct Config{
        VkExtent2D swapchainExtent;
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
    };

    explicit ViewportConfig(const VkExtent2D& extent);

    ViewportConfig& addViewport(const VkViewport& viewport);
    ViewportConfig& addScissor(const VkRect2D& scissor);

    const VkPipelineViewportStateCreateInfo& getCreateInfo() const;

private:
    Config m_config;
    VkPipelineViewportStateCreateInfo m_createInfo{};
};