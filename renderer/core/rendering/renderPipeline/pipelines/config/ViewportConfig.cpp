#include "ViewportConfig.hpp"

ViewportConfig::ViewportConfig(const VkExtent2D& extent) {
    m_config.swapchainExtent = extent;
    // 默认全屏视口
    addViewport({
        0.0f, 0.0f,
        static_cast<float>(extent.width),
        static_cast<float>(extent.height),
        0.0f, 1.0f
        });
    addScissor({ {0, 0}, extent });
}

ViewportConfig& ViewportConfig::addViewport(const VkViewport& viewport) {
    m_config.viewports.push_back(viewport);
    return *this;
}

ViewportConfig& ViewportConfig::addScissor(const VkRect2D& scissor) {
    m_config.scissors.push_back(scissor);
    return *this;
}

const VkPipelineViewportStateCreateInfo& ViewportConfig::getCreateInfo() const {
    static VkPipelineViewportStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.viewportCount = static_cast<uint32_t>(m_config.viewports.size());
    info.pViewports = m_config.viewports.data();
    info.scissorCount = static_cast<uint32_t>(m_config.scissors.size());
    info.pScissors = m_config.scissors.data();
    return info;
}