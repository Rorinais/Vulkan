#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include "./ResourceManager/ResourceManager.hpp"
#include "../../passes/VulkanRenderPass.hpp"
#include "../VulkanCore/VulkanCore.hpp"
#include "../WindowContext/WindowContext.hpp"

class RenderGraph {
public:
    using Ptr = std::shared_ptr<RenderGraph>;
    static Ptr create() { return std::make_shared<RenderGraph>(); }

    ~RenderGraph();

    void addPass(RenderPass::Ptr pass);
    void setMainPass(RenderPass::Ptr pass);
    void setResourceManager(ResourceManager::Ptr manager);

    void compile(VulkanCore::Ptr core, WindowContext::Ptr wContext);
    void execute(CommandBuffer::Ptr cmdBuffer, uint32_t imageIndex, uint32_t frameIndex);
    void onSwapchainRecreated();

    ResourceManager::Ptr getResourceManager() const { return mResourceManager; }

private:
    struct PassNode {
        RenderPass::Ptr pass;
        std::vector<size_t> dependencies;
        std::vector<size_t> dependents;
    };

    void buildDependencies();
    void topologicalSort();

    std::vector<PassNode> mPasses;
    std::vector<size_t> mExecutionOrder;
    ResourceManager::Ptr mResourceManager;
};