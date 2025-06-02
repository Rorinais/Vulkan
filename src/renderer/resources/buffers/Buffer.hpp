#pragma once
#include "../../../base.hpp"
#include "../../core/context/logicalDevice.hpp"
#include "../../core/commands/commandPool.hpp"

class Buffer {
public:
    using Ptr = std::shared_ptr<Buffer>;
    static Ptr create(const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool) {
        return std::make_shared<Buffer>(logicalDevice, commandPool);
    }

    Buffer(
        const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool
    ) : mLogicalDevice(logicalDevice), mCommandPool(commandPool) {}

    virtual ~Buffer() { cleanup(); }

    const VkBuffer& getBuffer() const noexcept { return mBuffer; }
    const VkDeviceMemory& getMemory() const noexcept { return mBufferMemory; }

    VkBuffer& getBuffer() noexcept { return mBuffer; }
    VkDeviceMemory& getMemory() noexcept { return mBufferMemory; }
    VkDeviceSize getSize() const noexcept { return mBufferSize; }
    VkDeviceSize getAlignedSize() const noexcept {return mAlignedSize;}

    virtual void cleanup() noexcept {
        if (mBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(mLogicalDevice->getHandle(), mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;
        }
        if (mBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mLogicalDevice->getHandle(), mBufferMemory, nullptr);
            mBufferMemory = VK_NULL_HANDLE;
        }
        mBufferSize = 0;
    }

    void* map() {
        if (!mMapped) {
            VkResult result = vkMapMemory(mLogicalDevice->getHandle(), mBufferMemory, 0, mBufferSize, 0, &mMapped);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to map buffer memory");
            }
        }
        return mMapped;
    }

    void unmap() {
        if (mMapped) {
            vkUnmapMemory(mLogicalDevice->getHandle(), mBufferMemory);
            mMapped = nullptr;
        }
    }

    void createBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties);

    template <typename DataType>
    void uploadData(const DataType* data, VkDeviceSize dataSize, VkBufferUsageFlags finalUsage);


    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

protected:
    LogicalDevice::Ptr mLogicalDevice;
    CommandPool::Ptr mCommandPool;

    VkBuffer mBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize mBufferSize = 0;
    VkDeviceSize mAlignedSize;
    void* mMapped = nullptr;
};

// 带暂存缓冲区的数据上传
template <typename DataType>
void Buffer::uploadData(const DataType* data, VkDeviceSize dataSize, VkBufferUsageFlags finalUsage) {
    // 创建暂存缓冲区
    Buffer stagingBuffer(mLogicalDevice, mCommandPool);
    stagingBuffer.createBuffer(
        dataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    // 映射并拷贝数据
    void* mapped;
    vkMapMemory(mLogicalDevice->getHandle(), stagingBuffer.mBufferMemory, 0, dataSize, 0, &mapped);
    memcpy(mapped, data, dataSize);
    vkUnmapMemory(mLogicalDevice->getHandle(), stagingBuffer.mBufferMemory);

    // 创建目标缓冲区
    createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | finalUsage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 执行拷贝命令
    copyBuffer(stagingBuffer.mBuffer, mBuffer, dataSize);
}

