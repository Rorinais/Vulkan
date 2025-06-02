#include "ResourceFactory.hpp"
#include <stdexcept>

ResourceFactory::UniformBufferData ResourceFactory::createUniformBuffer(
    const LogicalDevice::Ptr& logicalDevice,
    size_t dataSize,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceProperties physDevProps{};
    vkGetPhysicalDeviceProperties(logicalDevice->getPhysicalDevice()->getHandle(), &physDevProps);
    VkDeviceSize alignment = physDevProps.limits.minUniformBufferOffsetAlignment;
    if (alignment > 0)
        dataSize = (dataSize + alignment - 1) & ~(alignment - 1);

    UniformBufferData bufferData{};
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice->getHandle(), &bufferInfo, nullptr,
        &bufferData.buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create uniform buffer");

    VkMemoryRequirements memReqs{};
    vkGetBufferMemoryRequirements(logicalDevice->getHandle(),
        bufferData.buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(logicalDevice, memReqs.memoryTypeBits, properties);

    if (vkAllocateMemory(logicalDevice->getHandle(), &allocInfo, nullptr,
        &bufferData.memory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate uniform buffer memory");

    vkBindBufferMemory(logicalDevice->getHandle(), bufferData.buffer,
        bufferData.memory, 0);

    if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vkMapMemory(logicalDevice->getHandle(), bufferData.memory, 0,
            dataSize, 0, &bufferData.mapped);
    }

    bufferData.size = dataSize;
    return bufferData;
}

uint32_t ResourceFactory::findMemoryType(
    const LogicalDevice::Ptr& logicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(logicalDevice->getPhysicalDevice()->getHandle(), &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("Failed to find suitable memory type");
}