#pragma once
#include "Buffer.hpp"

class VertexBuffer : public Buffer {
public:
    using Ptr = std::shared_ptr<VertexBuffer>;
    static Ptr create(const LogicalDevice::Ptr& logicalDevice, const CommandPool::Ptr& commandPool);

    VertexBuffer(const LogicalDevice::Ptr& logicalDevice,const CommandPool::Ptr& commandPool);

    void uploadData(const void* data, VkDeviceSize size);
};