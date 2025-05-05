#include "ViewportConfig.hpp"
#include "../../../../base.hpp"

void ViewportConfig::createViewport(const VkExtent2D& extent) {
    // 核心参数计算
    float width = static_cast<float>(extent.width);
    float height = USE_OPENGL_COORDINATES
        ? -static_cast<float>(extent.height)  // 高度取负
        : static_cast<float>(extent.height);

    float viewportY = USE_OPENGL_COORDINATES
        ? static_cast<float>(extent.height)   // 原点设在物理屏幕底部
        : 0.0f;

    // 创建视口
    addViewport({
        0.0f,
        viewportY,
        width,       
        height,
        0.0f,        
        1.0f         
        });

    // 裁剪区域必须使用正向尺寸
    addScissor({
        {0, 0},      // offset
        {extent.width, extent.height} // 始终使用正数尺寸
        });
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
    // 调试输出
    //std::cout << "=== Viewport Configuration ===" << "\n"
    //    << "Viewport Count: " << m_config.viewports.size() << "\n"
    //    << "Viewport[0]:\n"
    //    << "  x: " << m_config.viewports[0].x << "\n"
    //    << "  y: " << m_config.viewports[0].y << "\n"
    //    << "  width: " << m_config.viewports[0].width << "\n"
    //    << "  height: " << m_config.viewports[0].height << "\n"
    //    << "Scissor[0]:\n"
    //    << "  offset: (" << m_config.scissors[0].offset.x
    //    << ", " << m_config.scissors[0].offset.y << ")\n"
    //    << "  extent: " << m_config.scissors[0].extent.width
    //    << "x" << m_config.scissors[0].extent.height << "\n";

    // 返回创建信息
    static VkPipelineViewportStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.viewportCount = static_cast<uint32_t>(m_config.viewports.size());
    info.pViewports = m_config.viewports.data();
    info.scissorCount = static_cast<uint32_t>(m_config.scissors.size());
    info.pScissors = m_config.scissors.data();
    return info;
}