#pragma once
#include"../base.h"

class LogicalDevice {
public:
    struct QueueHandles {
        VkQueue graphics;
        VkQueue present;
    };

    LogicalDevice(VkPhysicalDevice physicalDevice,
        const QueueFamilyIndices& queueIndices,
        bool enableValidation = false);
    ~LogicalDevice();

    VkDevice getDevice() const { return mDevice; }
    QueueHandles getQueues() const { return mQueues; }

private:
    VkDevice mDevice = VK_NULL_HANDLE;
    QueueHandles mQueues{};
};