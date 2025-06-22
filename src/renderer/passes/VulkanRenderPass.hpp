#pragma once
#include "../passes/RenderPass.hpp"
#include "../pipeline/pipeline.hpp"
#include "../resources/models/mesh/Mesh.hpp"
#include "../core/RenderGraph/RenderGraph.hpp"

class ShaderProgram;

class VulkanRenderPass : public RenderPass {
public:
    struct AttachmentConfig {
        VkFormat format;
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
        VkAttachmentLoadOp loadOp;
        VkAttachmentStoreOp storeOp;
        bool isDepth;
    };

    struct DependencyConfig {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
    };

    using Ptr = std::shared_ptr<VulkanRenderPass>;
    static Ptr create(Type type) {
        return std::make_shared<VulkanRenderPass>(type);
    }

    explicit VulkanRenderPass(Type type);
    ~VulkanRenderPass() override;

    VulkanRenderPass& addAttachment(const AttachmentConfig& config);
    VulkanRenderPass& addDependency(const DependencyConfig& config);

    void init(VulkanCore::Ptr vulkanCore, WindowContext::Ptr windowContext) override;
    void buildPipeline();
    void createPipeline();
    void createFramebuffers();
    void build() override;
    void destroy() override;
    void onSwapChainRecreated() override;

    void beginFrame(uint32_t imageIndex) override;
    void recordCommands(CommandBuffer::Ptr cmdBuffer, uint32_t imageIndex, uint32_t frameIndex) override;
    void endFrame() override;

    void setShaderProgram(ShaderProgram::Ptr shader) { 
        mShaderProgram = shader; 
    }

    void setMesh(const Mesh& mesh) { 
        mMesh = mesh; 
    }

    VkFramebuffer getFramebuffer(uint32_t imageIndex) const override {
        return mSwapchainFramebuffers.at(imageIndex);
    }

    VkRenderPass getHandle() const override { 
        return mRenderPass; 
    }

    void setResourceManager(ResourceManager::Ptr manager) {
        mResourceManager = manager;
    }

    void setDescriptorSets(const std::vector<VkDescriptorSet>& descriptorSets) {
        mDescriptorSets = descriptorSets;
    }

    void setDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout>& layouts) {
        mDescriptorSetLayouts = layouts;
    }
   
private:

    std::vector<AttachmentConfig> mAttachmentConfigs;
    std::vector<DependencyConfig> mDependencyConfigs;

    VulkanCore::Ptr mVulkanCore;
    WindowContext::Ptr mWindowContext;

    Pipeline::Ptr mPipeline;
    ShaderProgram::Ptr mShaderProgram;
    Mesh mMesh;
    ResourceManager::Ptr mResourceManager;

    std::vector<VkFramebuffer> mSwapchainFramebuffers;

    std::vector<VkDescriptorSet> mDescriptorSets;
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
};