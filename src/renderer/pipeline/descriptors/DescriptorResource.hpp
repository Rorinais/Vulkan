#pragma once
#include "../../../base.hpp"
#include "ResourceFactory.hpp"
#include "../../resources/textures/Texture.hpp"
#include <string>
#include <memory>

class DescriptorResource {
public:
    using Ptr = std::shared_ptr<DescriptorResource>;

    enum class Type { UniformBuffer, CombinedImageSampler };

    DescriptorResource(const LogicalDevice::Ptr& logicalDevice, Type type)
        : mLogicalDevice(logicalDevice), mType(type) {
    }

    virtual ~DescriptorResource() = default;

    Type getType() const { return mType; }
    LogicalDevice::Ptr getLogicalDevice() const { return mLogicalDevice; }

private:
    LogicalDevice::Ptr mLogicalDevice;
    Type mType;
};

class UniformBufferResource : public DescriptorResource {
public:
    using UniformBufferData = ResourceFactory::UniformBufferData;
    using Ptr = std::shared_ptr<UniformBufferResource>;

    UniformBufferResource(const LogicalDevice::Ptr& logicalDevice, size_t dataSize);
    ~UniformBufferResource();

    void update(const void* data, size_t size);
    const UniformBufferData& getData() const { return mData; }

private:
    void cleanup();
    UniformBufferData mData;
};

class TextureResource : public DescriptorResource {
public:
    using Ptr = std::shared_ptr<TextureResource>;

    TextureResource(const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool,
        const char* imagePath);

    Texture::Ptr getTexture() const { return mTexture; }

private:
    Texture::Ptr mTexture;
};