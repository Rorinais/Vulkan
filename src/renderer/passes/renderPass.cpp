#include "renderPass.hpp"

RenderPass::RenderPass(VkDevice logicalDevice)
    : mLogicalDevice(logicalDevice)
{
    setupDefaultDependencies();
}

RenderPass::~RenderPass() {
    if (mRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(mLogicalDevice, mRenderPass, nullptr);
    }
}

RenderPass& RenderPass::addAttachment(const AttachmentConfig& config) {
    AttachmentInfo info;
    info.type = config.type;

    info.desc = {
        .format = config.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = config.loadOp,
        .storeOp = config.storeOp,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = config.initialLayout,
        .finalLayout = config.finalLayout
    };

    switch (config.type) {
    case Type::Color:
        info.ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
    case Type::Depth:
        info.ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        break;
    }

    info.ref.attachment = static_cast<uint32_t>(mAttachments.size());
    mAttachments.push_back(info);
    return *this;
}

RenderPass& RenderPass::addDependency(const DependencyConfig& config) {
    mDependencies.push_back(VkSubpassDependency{
        .srcSubpass = config.srcSubpass,
        .dstSubpass = config.dstSubpass,
        .srcStageMask = config.srcStageMask,
        .dstStageMask = config.dstStageMask,
        .srcAccessMask = config.srcAccessMask,
        .dstAccessMask = config.dstAccessMask,
        .dependencyFlags = config.dependencyFlags
        });
    return *this;
}

void RenderPass::setupDefaultDependencies() {
    addDependency(DependencyConfig{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        });
}

void RenderPass::validateAttachments() const {
    if (mAttachments.empty()) {
        throw std::runtime_error("At least one attachment required");
    }

    const size_t depthCount = std::count_if(mAttachments.begin(), mAttachments.end(),
        [](const auto& a) { return a.type == Type::Depth; });

    if (depthCount > 1) {
        throw std::runtime_error("Multiple depth attachments not supported");
    }
}

void RenderPass::build() {
    validateAttachments();

    std::vector<VkAttachmentDescription> attachments;
    for (const auto& a : mAttachments) {
        attachments.push_back(a.desc);
    }

    // 组织附件引用
    std::vector<VkAttachmentReference> colorRefs;
    VkAttachmentReference depthRef{};
    bool hasDepth = false;

    for (const auto& a : mAttachments) {
        if (a.type == Type::Color) {
            colorRefs.push_back(a.ref);
        }
        else {
            depthRef = a.ref;
            hasDepth = true;
        }
    }

    // 创建子流程描述
    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(colorRefs.size()),
        .pColorAttachments = colorRefs.data(),
        .pDepthStencilAttachment = hasDepth ? &depthRef : nullptr
    };

    // 创建渲染流程
    const VkRenderPassCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(mDependencies.size()),
        .pDependencies = mDependencies.data()
    };

    if (vkCreateRenderPass(mLogicalDevice, &createInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}