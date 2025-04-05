// RenderPassBuilder.cpp
#include "RenderPassBuilder.hpp"
#include <stdexcept>

RenderPassBuilder::RenderPassBuilder(VkDevice device)
    : m_device(device) {
    m_mainSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // 默认子流程依赖
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_dependencies.push_back(dependency);
}

RenderPassBuilder& RenderPassBuilder::configureColorAttachment(const RenderPassConfig& config) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = config.colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = config.loadOp;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = config.initialLayout;
    colorAttachment.finalLayout = config.finalLayout;
    m_attachments.push_back(colorAttachment);

    VkAttachmentReference colorRef{};
    colorRef.attachment = static_cast<uint32_t>(m_attachments.size() - 1);
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_colorRefs.push_back(colorRef);

    // 更新主子流程配置
    m_mainSubpass.colorAttachmentCount = static_cast<uint32_t>(m_colorRefs.size());
    m_mainSubpass.pColorAttachments = m_colorRefs.data();

    return *this;
}

VkRenderPass RenderPassBuilder::build() const {
    if (m_attachments.empty()) {
        throw std::runtime_error("No attachments configured");
    }

    std::vector<VkSubpassDescription> subpasses = { m_mainSubpass };

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
    createInfo.pAttachments = m_attachments.data();
    createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    createInfo.pSubpasses = subpasses.data();
    createInfo.dependencyCount = static_cast<uint32_t>(m_dependencies.size());
    createInfo.pDependencies = m_dependencies.data();

    VkRenderPass renderPass;
    if (vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
    return renderPass;
}