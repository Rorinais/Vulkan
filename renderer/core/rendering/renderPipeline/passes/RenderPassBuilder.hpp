// RenderPassBuilder.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class RenderPassBuilder {
public:
    struct RenderPassConfig {
        VkFormat colorFormat;
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    };

    explicit RenderPassBuilder(VkDevice device);
    RenderPassBuilder& configureColorAttachment(const RenderPassConfig& config);
    VkRenderPass build() const;

private:
    VkDevice m_device;
    std::vector<VkAttachmentDescription> m_attachments;
    std::vector<VkAttachmentReference> m_colorRefs;
    VkSubpassDescription m_mainSubpass{};
    std::vector<VkSubpassDependency> m_dependencies;
};