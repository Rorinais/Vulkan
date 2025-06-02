#pragma once
#include "../../base.hpp"
#include"../core/context/logicalDevice.hpp"

class RenderPass {
public:
    enum class Type {
        Color,
        Depth
    };

    struct DependencyConfig {
        uint32_t srcSubpass;
        uint32_t dstSubpass;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
        VkDependencyFlags dependencyFlags = 0;
    };

    struct AttachmentConfig {
        Type type;
        VkFormat format;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout;
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    };

    using Ptr = std::shared_ptr<RenderPass>;
    static Ptr create(VkDevice logicalDevice) { return std::make_shared<RenderPass>(logicalDevice); }

    RenderPass(VkDevice logicalDevice);
    ~RenderPass();

    RenderPass& addAttachment(const AttachmentConfig& config);
    RenderPass& addDependency(const DependencyConfig& config);
    void build();

    VkRenderPass getHandle() const { return mRenderPass; }

private:
    void setupDefaultDependencies();
    void validateAttachments() const;

    struct AttachmentInfo {
        VkAttachmentDescription desc;
        VkAttachmentReference ref;
        Type type;
    };

    VkDevice mLogicalDevice;
    std::vector<AttachmentInfo> mAttachments;
    std::vector<VkSubpassDependency> mDependencies;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
};