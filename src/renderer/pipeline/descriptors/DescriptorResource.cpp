#include "DescriptorResource.hpp"

UniformBufferResource::UniformBufferResource(const LogicalDevice::Ptr& logicalDevice,
    const CommandPool::Ptr& commandPool,
    size_t dataSize)
    : DescriptorResource(logicalDevice, Type::UniformBuffer),
    mCommandPool(commandPool),
    mData(UniformBuffer::create(logicalDevice, commandPool, dataSize))
{
}

UniformBufferResource::~UniformBufferResource() {
    // Cleanup handled by smart pointers
}

void UniformBufferResource::update(const void* data, size_t size) {
    if (size > mData->getSize()) {
        throw std::runtime_error("Data size exceeds buffer capacity");
    }
    mData->uploadData(data, size, 0);
}

TextureResource::TextureResource(const LogicalDevice::Ptr& logicalDevice,
    const CommandPool::Ptr& commandPool,
    const char* imagePath)
    : DescriptorResource(logicalDevice, Type::CombinedImageSampler),
    mTexture(Texture::create(logicalDevice, commandPool, imagePath))
{
}