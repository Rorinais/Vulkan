#pragma once
#include <vulkan/vulkan.h>

#include "../../ShaderManager.hpp"
#include "../passes/RenderPassBuilder.hpp"
#include "config/VertexInputConfig.hpp"
#include "config/InputAssemblyConfig.hpp"
#include "config/ViewportConfig.hpp"
#include "config/RasterizationConfig.hpp"
#include "config/MultisampleConfig.hpp"
#include "config/ColorBlendConfig.hpp"
#include <memory>

class PipelineFactory {
public:
    struct PipelineConfig {
        struct {
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
            VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
            VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
        } rasterization;

        struct {
            VkBool32 blendEnable = VK_FALSE;
            VkBlendFactor srcFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        } blending;
    };

    PipelineFactory(VkDevice device);

    void configure(const PipelineConfig& config);

    VkPipeline createGraphicsPipeline(VkRenderPass renderPass,VkExtent2D swapchainExtent,VkShaderModule vertShaderPath,VkShaderModule fragShaderPath);

    VkPipelineLayout getPipelineLayout() { return mPipelineLayout; }

private:
    VkDevice m_device;
    PipelineConfig m_config;
    VkPipelineLayout mPipelineLayout=VK_NULL_HANDLE;
};

