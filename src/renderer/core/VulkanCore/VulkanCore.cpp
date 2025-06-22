#include "VulkanCore.hpp"
#include "../WindowContext/WindowContext.hpp"

VulkanCore::VulkanCore() {}

VulkanCore::~VulkanCore() {
    cleanup();
}

bool VulkanCore::init(Window::Ptr window) {
    instance = Instance::create({ "Vulkan App" });
    vulkanDebug = VulkanDebug::create(instance);

    createSurface(window);

    physicalDevice = PhysicalDevice::create(instance, mSurface);

    LogicalDevice::Config deviceConfig{};
    deviceConfig.samplerAnisotropy = VK_TRUE;
    deviceConfig.fillModeNonSolid = VK_TRUE;
    deviceConfig.wideLines = VK_TRUE;
    logicalDevice = LogicalDevice::create(physicalDevice, deviceConfig);

    mInitialized = true;
    return true;
}

bool VulkanCore::isInitialized() const {
    return mInitialized;
}

void VulkanCore::cleanup() {
    logicalDevice.reset();
    physicalDevice.reset();
    vulkanDebug.reset();

    if (mSurface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance->getHandle(), mSurface, nullptr);
        mSurface = VK_NULL_HANDLE;
    }

    instance.reset();
}

void VulkanCore::createSurface(Window::Ptr window) {
    if (glfwCreateWindowSurface(
        instance->getHandle(),
        window->getHandle(),
        nullptr,
        &mSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}