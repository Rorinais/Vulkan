#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class ViewportConfig {
public:
    struct Viewport {
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
    };

	ViewportConfig() = default;

    void createViewport(const VkExtent2D& extent);

    ViewportConfig& addViewport(const VkViewport& viewport);
    ViewportConfig& addScissor(const VkRect2D& scissor);

    const VkPipelineViewportStateCreateInfo& getCreateInfo() const;

private:
    Viewport m_config;
    VkPipelineViewportStateCreateInfo m_createInfo{};
};