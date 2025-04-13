#pragma once
#include <vulkan/vulkan.h>

#include "config/VertexInputConfig.hpp"
#include "config/InputAssemblyConfig.hpp"
#include "config/ViewportConfig.hpp"
#include "config/RasterizationConfig.hpp"
#include "config/MultisampleConfig.hpp"
#include "config/ColorBlendConfig.hpp"
#include "../../ShaderManager.hpp"
#include "../passes/RenderPassBuilder.hpp"
#include <memory>

class PipelineFactory {
public:
    struct PipelineConfig {
        VertexInputConfig vertexInput;
        InputAssemblyConfig inputAssembly;
        ViewportConfig viewport;
        RasterizationConfig rasterization;
        MultisampleConfig multisample;
        ColorBlendConfig colorBlend;
    };

    PipelineFactory(VkDevice device);
    ~PipelineFactory();

    void configure(const PipelineConfig& config);

    void createGraphicsPipeline(VkShaderModule vertShaderPath,VkShaderModule fragShaderPath);

    void createRenderPass(VkFormat SwapChainImageFormat);

    void cleanup();

    VkPipelineLayout& getPipelineLayout() { return mPipelineLayout; }
    VkRenderPass& getRenderPass() { return mRenderPass; }
    VkPipeline& getGraphicsPipeline() { return mGraphicsPipeline; }

private:
    PipelineConfig mConfig;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
};

