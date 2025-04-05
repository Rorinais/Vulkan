#pragma once
#include "../base.h"

class PhysicalDeviceSelector {
public:
    PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface);

    VkPhysicalDevice select() const;
    QueueFamilyIndices getQueueIndices(VkPhysicalDevice device) const;
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
private:
    bool isSuitable(VkPhysicalDevice device) const;
    bool checkExtensions(VkPhysicalDevice device) const;

    VkInstance mInstance;
    VkSurfaceKHR m_surface;
};