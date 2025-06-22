#pragma once
#include "SwapChain.hpp"
#include "../VulkanCore/VulkanCore.hpp"
#include "../../resources/textures/Texture.hpp"
#include "../FrameContext/FrameContext.hpp"

class WindowContext {
public:
    using Ptr = std::shared_ptr<WindowContext>;
    static Ptr create() { return std::make_shared<WindowContext>(); }

    ~WindowContext() {
        mSwapChain.reset();
		mDepthTexture.reset();  
    }


    bool init(VulkanCore::Ptr vulkanCore, Window::Ptr window, CommandPool::Ptr commandPool);
    void cleanupSwapchain();
    void recreateSwapchain();
    void onSwapchainRecreated();

    SwapChain::Ptr getSwapChain() const { return mSwapChain; }
	CommandPool::Ptr getCommandPool() const { return mCommandPool; }
    Texture::Ptr getDepthTexture() const { return mDepthTexture; }
    VkExtent2D getSwapchainExtent() const { return mSwapChain->getExtent(); }
    VkFormat getSwapchainFormat() const { return mSwapChain->getImageFormat(); }
    const std::vector<VkImageView>& getSwapchainImageViews() const {
        return mSwapChain->getImageViews();
    }
    uint32_t getSwapchainImageCount() const { return mSwapChain->getImageCount(); }
    VkSurfaceKHR getSurface() const { return mVulkanCore->getSurface(); }
    bool isInitialized() const;
private:
    bool mInitialized = false;
    void createDepthResources();

    VulkanCore::Ptr mVulkanCore;
    Window::Ptr mWindow;
    CommandPool::Ptr mCommandPool;
    SwapChain::Ptr mSwapChain;
    Texture::Ptr mDepthTexture;
};