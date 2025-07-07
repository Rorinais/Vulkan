#include "WindowContext.hpp"

bool WindowContext::init(VulkanCore::Ptr vulkanCore, Window::Ptr window, CommandPool::Ptr commandPool) {
    mVulkanCore = vulkanCore;
    mWindow = window;
    mCommandPool = commandPool;

    // 创建交换链
    mSwapChain = SwapChain::create(
        mVulkanCore->getLogicalDevice(),
        mVulkanCore->getSurface(),
        mWindow
    );

    createDepthResources();
    mInitialized = true;
    return true;
}

void WindowContext::cleanupSwapchain() {
    if (mDepthTexture) {
        mDepthTexture->cleanup();  
        mDepthTexture.reset();     
    }

    if (mSwapChain) {
        mSwapChain->cleanup();
        mSwapChain.reset();
    }
}

void WindowContext::recreateSwapchain() {
    cleanupSwapchain();

    mSwapChain = SwapChain::create(
        mVulkanCore->getLogicalDevice(),
        mVulkanCore->getSurface(),
        mWindow
    );

    createDepthResources();
}

void WindowContext::createDepthResources() {
    if (mDepthTexture) {
        mDepthTexture.reset();
    }
    mDepthTexture = Texture::create(
        mVulkanCore->getLogicalDevice(),
        Texture::Type::Depth,
        VkExtent2D{
            mSwapChain->getExtent().width,
            mSwapChain->getExtent().height
        },
        mCommandPool
    );
}

void WindowContext::onSwapchainRecreated() {
    cleanupSwapchain();
    recreateSwapchain();
}

bool WindowContext::isInitialized() const {
    return mInitialized;
}