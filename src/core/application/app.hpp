#pragma once
#include "../../renderer/core/VulkanCore/VulkanCore.hpp"
#include "../../renderer/core/WindowContext/WindowContext.hpp"
#include "../../renderer/core/VulkanRenderer.hpp"

class App {
public:
    App();
    ~App();
    void run();

private:
    Window::Ptr window;
    VulkanCore::Ptr vulkanCore;
    WindowContext::Ptr windowContext;
    VulkanRenderer::Ptr renderer;

    bool mFramebufferResized = false;
};