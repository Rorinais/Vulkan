#include "DescriptorResource.hpp"

UniformBufferResource::UniformBufferResource(const LogicalDevice::Ptr& logicalDevice, size_t dataSize)
    : DescriptorResource(logicalDevice, Type::UniformBuffer),
    mData(ResourceFactory::createUniformBuffer(logicalDevice, dataSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
{
}

UniformBufferResource::~UniformBufferResource() {
    cleanup();
}

void UniformBufferResource::cleanup() {
    auto device = getLogicalDevice()->getHandle();
    if (mData.mapped) {
        vkUnmapMemory(device, mData.memory);
        mData.mapped = nullptr;
    }
    if (mData.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, mData.buffer, nullptr);
        mData.buffer = VK_NULL_HANDLE;
    }
    if (mData.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, mData.memory, nullptr);
        mData.memory = VK_NULL_HANDLE;
    }
    mData.size = 0;
}

void UniformBufferResource::update(const void* data, size_t size) {
    if (size > mData.size) {
        throw std::runtime_error("Data size exceeds buffer capacity");
    }
    memcpy(mData.mapped, data, size);
}

TextureResource::TextureResource(const LogicalDevice::Ptr& logicalDevice,
    const CommandPool::Ptr& commandPool,
    const char* imagePath)
    : DescriptorResource(logicalDevice, Type::CombinedImageSampler),
    mTexture(Texture::create(logicalDevice, commandPool, imagePath))
{
}