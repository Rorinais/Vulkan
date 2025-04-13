#pragma once
#include "../base.hpp"

class PhysicalDeviceSelector {
public:
    PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface);

    VkSurfaceKHR getSurface() { return mSurface; }

    VkPhysicalDevice getPhysicalDevice() { return mPhysicalDevice; }

    QueueFamilyIndices getQueueIndices(VkPhysicalDevice device) const;
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
private:
    VkPhysicalDevice select() const;
    bool isSuitable(VkPhysicalDevice device) const;
    bool checkExtensions(VkPhysicalDevice device) const;

    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
};