#include "RenderGraph.hpp"
#include "../VulkanCore/VulkanCore.hpp"
#include "../WindowContext/WindowContext.hpp"

RenderGraph::~RenderGraph() {
    // 确保所有资源正确释放
    mResourceManager.reset();
    mPasses.clear();
    mExecutionOrder.clear();
}

void RenderGraph::addPass(RenderPass::Ptr pass) {
    if (!pass) {
        throw std::runtime_error("Attempted to add null pass to RenderGraph");
    }

    PassNode node{ pass, {}, {} };
    mPasses.push_back(std::move(node));
}

void RenderGraph::setMainPass(RenderPass::Ptr pass) {
    if (!pass) {
        throw std::runtime_error("Attempted to set null main pass");
    }

    // 确保主通道存在
    bool found = false;
    for (auto& node : mPasses) {
        if (node.pass == pass) {
            found = true;
            break;
        }
    }

    if (!found) {
        addPass(pass);
    }
}

void RenderGraph::setResourceManager(ResourceManager::Ptr manager) {
    mResourceManager = manager;
}

void RenderGraph::compile(VulkanCore::Ptr core, WindowContext::Ptr wContext) {

    if (!core || !core->isInitialized()) {
        throw std::runtime_error("VulkanCore not initialized when compiling RenderGraph");
    }

    if (!wContext || !wContext->isInitialized()) {
        throw std::runtime_error("WindowContext not initialized when compiling RenderGraph");
    }

    // 注册交换链资源
    mResourceManager->registerSwapchainResources(
        wContext->getSwapChain(),
        wContext->getSwapchainImageViews(),
        wContext->getDepthTexture()
    );

    // 构建依赖关系
    buildDependencies();

    // 拓扑排序确定执行顺序
    topologicalSort();

    // 初始化所有通道
    for (auto& node : mPasses) {
        node.pass->init(core, wContext);
        if (auto vkPass = std::dynamic_pointer_cast<VulkanRenderPass>(node.pass)) {
            vkPass->setResourceManager(mResourceManager);
        }
    }

    // 构建资源
    try {
        mResourceManager->buildResources();
        mResourceManager->buildDescriptorResources();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("ResourceManager build failed: " + std::string(e.what()));
    }

    // 3. 设置描述符集并创建管线
    for (auto& node : mPasses) {
        if (auto vkPass = std::dynamic_pointer_cast<VulkanRenderPass>(node.pass)) {
            vkPass->setDescriptorSetLayouts({ mResourceManager->getDescriptorSetLayout() });
            vkPass->setDescriptorSets(mResourceManager->getDescriptorSets());
            vkPass->buildPipeline(); 
        }
    }

    // 4. 创建帧缓冲（依赖管线）
    for (auto& node : mPasses) {
        if (auto vkPass = std::dynamic_pointer_cast<VulkanRenderPass>(node.pass)) {
            vkPass->createFramebuffers();
        }
    }
}

void RenderGraph::buildDependencies() {
    for (size_t i = 0; i < mPasses.size(); ++i) {
        auto& pass = mPasses[i].pass;

        switch (pass->getType()) {
        case RenderPass::SHADOW:
            // 阴影通道可能依赖主通道
            for (size_t j = 0; j < mPasses.size(); ++j) {
                if (mPasses[j].pass->getType() == RenderPass::MAIN) {
                    mPasses[i].dependencies.push_back(j);
                    mPasses[j].dependents.push_back(i);
                }
            }
            break;

        case RenderPass::POST_PROCESS:
            // 后处理依赖主通道
            for (size_t j = 0; j < mPasses.size(); ++j) {
                if (mPasses[j].pass->getType() == RenderPass::MAIN) {
                    mPasses[i].dependencies.push_back(j);
                    mPasses[j].dependents.push_back(i);
                }
            }
            break;

        default:
            break;
        }
    }
}

void RenderGraph::topologicalSort() {
    mExecutionOrder.clear();
    std::vector<size_t> inDegree(mPasses.size(), 0);
    std::queue<size_t> queue;

    // 计算入度
    for (size_t i = 0; i < mPasses.size(); ++i) {
        inDegree[i] = mPasses[i].dependencies.size();
        if (inDegree[i] == 0) {
            queue.push(i);
        }
    }

    // 拓扑排序
    while (!queue.empty()) {
        size_t current = queue.front();
        queue.pop();
        mExecutionOrder.push_back(current);

        for (size_t dependent : mPasses[current].dependents) {
            if (--inDegree[dependent] == 0) {
                queue.push(dependent);
            }
        }
    }

    // 检查环
    if (mExecutionOrder.size() != mPasses.size()) {
        throw std::runtime_error("RenderGraph has cyclic dependencies");
    }
}

void RenderGraph::execute(CommandBuffer::Ptr cmdBuffer, uint32_t imageIndex, uint32_t frameIndex) {
    if (!cmdBuffer || !cmdBuffer->isRecording()) {
        throw std::runtime_error("Command buffer not in recording state");
    }

    // 按排序后的顺序执行通道
    for (size_t idx : mExecutionOrder) {
        auto& node = mPasses[idx];

        if (!node.pass) {
            throw std::runtime_error("Null pass encountered during execution");
        }

        // 直接在主命令缓冲区记录渲染命令
        node.pass->recordCommands(cmdBuffer, imageIndex,frameIndex);
    }
}

void RenderGraph::onSwapchainRecreated() {
    if (!mResourceManager) {
        throw std::runtime_error("ResourceManager not initialized during swapchain recreation");
    }

    try {
        mResourceManager->onSwapchainRecreated();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("ResourceManager swapchain recreation failed: " + std::string(e.what()));
    }

    // 重新创建所有通道的帧缓冲
    for (auto& node : mPasses) {
        if (auto vkPass = std::dynamic_pointer_cast<VulkanRenderPass>(node.pass)) {
            vkPass->onSwapChainRecreated();
        }
    }
}