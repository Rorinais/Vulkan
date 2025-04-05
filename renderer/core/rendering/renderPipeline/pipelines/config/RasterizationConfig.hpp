// include/rendering/config/RasterizationConfig.hpp
#pragma once
#include <vulkan/vulkan.h>

class RasterizationConfig {
public:
    struct Config {
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
        float lineWidth = 1.0f;
    };

    RasterizationConfig& setPolygonMode(VkPolygonMode mode);
    RasterizationConfig& setCullMode(VkCullModeFlags cullMode);
    RasterizationConfig& setFrontFace(VkFrontFace frontFace);

    const VkPipelineRasterizationStateCreateInfo& getCreateInfo() const;

private:
    Config m_config;
    VkPipelineRasterizationStateCreateInfo m_createInfo{};
};