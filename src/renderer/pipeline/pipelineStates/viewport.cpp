#include "viewport.hpp"
Viewport& Viewport::addViewport(const VkViewport& viewport) {
    mViewports.viewports.push_back(viewport);
    return *this;
}

Viewport& Viewport::addScissor(const VkRect2D& scissor) {
    mViewports.scissors.push_back(scissor);
    return *this;
}

Viewport& Viewport::createViewport(const VkExtent2D& extent) {
    mViewports.viewports.clear();
    mViewports.scissors.clear();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = mIsOpenGLCoord ? static_cast<float>(extent.height) : 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = mIsOpenGLCoord ? -static_cast<float>(extent.height) : static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    mViewports.viewports.push_back(viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    mViewports.scissors.push_back(scissor);

    // 初始化结构体
    mCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    mCreateInfo.viewportCount = static_cast<uint32_t>(mViewports.viewports.size());
    mCreateInfo.pViewports = mViewports.viewports.data();
    mCreateInfo.scissorCount = static_cast<uint32_t>(mViewports.scissors.size());
    mCreateInfo.pScissors = mViewports.scissors.data();

    return *this;
}

const VkPipelineViewportStateCreateInfo& Viewport::getCreateInfo() const {
    return mCreateInfo;
}