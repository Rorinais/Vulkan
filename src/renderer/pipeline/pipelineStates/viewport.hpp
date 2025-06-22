#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class Viewport {
public:
    struct {
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;
    } mViewports;

    Viewport() = default;

    Viewport& createViewport(const VkExtent2D& extent);

    Viewport& addViewport(const VkViewport& viewport);
    Viewport& addScissor(const VkRect2D& scissor);

    Viewport& IsOpenglCoordinate(bool flag = true) { 
        mIsOpenGLCoord = flag; 
        return *this;
    }

    const VkPipelineViewportStateCreateInfo& getCreateInfo() const;

private:
    VkPipelineViewportStateCreateInfo mCreateInfo{};  
    bool mIsOpenGLCoord = false;
};