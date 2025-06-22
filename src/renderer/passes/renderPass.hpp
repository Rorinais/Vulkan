#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include "../core/VulkanCore/VulkanCore.hpp"
#include "../core/WindowContext/WindowContext.hpp"
#include "../core/FrameContext/FrameContext.hpp"
#include "../resources/models/mesh/Mesh.hpp"

// 前向声明
class CommandPool;
class CommandBuffer;
class ShaderProgram;

class RenderPass {
public:
    using Ptr = std::shared_ptr<RenderPass>;

    enum Type {
        MAIN,
        SHADOW,
        POST_PROCESS
    };

    explicit RenderPass(Type type) : mType(type) {}
    virtual ~RenderPass() = default;

    // 纯虚函数声明
    virtual void destroy() = 0;
    virtual void init(VulkanCore::Ptr vulkanCore, WindowContext::Ptr windowContext) = 0;
    virtual void beginFrame(uint32_t imageIndex) = 0;
    virtual void recordCommands(CommandBuffer::Ptr cmdBuffer, uint32_t imageIndex, uint32_t frameIndex) = 0;
    virtual void endFrame() = 0;
    virtual void onSwapChainRecreated() = 0;
    virtual void build() = 0;
    virtual VkFramebuffer getFramebuffer(uint32_t imageIndex) const = 0;
    virtual VkRenderPass getHandle() const = 0;

    Type getType() const { return mType; }

protected:
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    Type mType;
};