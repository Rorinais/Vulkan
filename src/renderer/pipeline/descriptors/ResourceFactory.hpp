#pragma once
#include "../../../base.hpp"
#include"../../core/context/logicalDevice.hpp"
#include <vulkan/vulkan.h>

class ResourceFactory {
public:
    struct UniformBufferData {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void* mapped = nullptr;
        size_t size = 0;
    };

    static UniformBufferData createUniformBuffer(
        const LogicalDevice::Ptr& logicalDevice,
        size_t dataSize,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties);

private:
    static uint32_t findMemoryType(
        const LogicalDevice::Ptr& logicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties);
};