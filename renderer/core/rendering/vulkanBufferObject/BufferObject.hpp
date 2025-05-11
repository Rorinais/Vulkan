#pragma once
#include "../../base.hpp"
#include <stdexcept>



class BufferObject {
public:
    BufferObject() = default;
    BufferObject(const BufferObject&) = delete;
    BufferObject(BufferObject&& other) noexcept;
    BufferObject& operator=(const BufferObject&) = delete;
    BufferObject& operator=(BufferObject&& other) noexcept;

    explicit BufferObject(const VulkanContext& context) : mContext(context) {}

    virtual ~BufferObject() { cleanup(); }

    // 返回句柄的 const 引用（推荐）
    const VkBuffer& getBuffer() const noexcept { return mBuffer; }
    const VkDeviceMemory& getMemory() const noexcept { return mBufferMemory; }

    const VulkanContext& getVkContext() const noexcept{ return mContext; }

    // 可选：如果需要修改句柄的非 const 版本
    VkBuffer& getBuffer() noexcept { return mBuffer; }
    VkDeviceMemory& getMemory() noexcept { return mBufferMemory; }
    VkDeviceSize getSize() const noexcept { return mBufferSize; }

    virtual void cleanup() noexcept {
        if (mBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(mContext.logicalDevice, mBuffer, nullptr);
            mBuffer = VK_NULL_HANDLE;
        }
        if (mBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mContext.logicalDevice, mBufferMemory, nullptr);
            mBufferMemory = VK_NULL_HANDLE;
        }
        mBufferSize = 0;
    }

    void* map() {
        if (!mMapped) {
            VkResult result = vkMapMemory(mContext.logicalDevice, mBufferMemory, 0, mBufferSize, 0, &mMapped);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to map buffer memory");
            }
        }
        return mMapped;
    }

    void unmap() {
        if (mMapped) {
            vkUnmapMemory(mContext.logicalDevice, mBufferMemory);
            mMapped = nullptr;
        }
    }

    void createBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties);

    template <typename DataType>
    void uploadData(const DataType* data, VkDeviceSize dataSize, VkBufferUsageFlags finalUsage);


    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

protected:
    VulkanContext mContext;
    VkBuffer mBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize mBufferSize = 0;
    void* mMapped = nullptr;
};

// BufferObject 移动构造函数
BufferObject::BufferObject(BufferObject&& other) noexcept
    : mContext(other.mContext),
    mBuffer(other.mBuffer),
    mBufferMemory(other.mBufferMemory),
    mBufferSize(other.mBufferSize),
    mMapped(other.mMapped) {
    other.mBuffer = VK_NULL_HANDLE;
    other.mBufferMemory = VK_NULL_HANDLE;
    other.mBufferSize = 0;
    other.mMapped = nullptr;
}

// 移动赋值运算符
BufferObject& BufferObject::operator=(BufferObject&& other) noexcept {
    if (this != &other) {
        cleanup();
        mContext = other.mContext;
        mBuffer = other.mBuffer;
        mBufferMemory = other.mBufferMemory;
        mBufferSize = other.mBufferSize;
        mMapped = other.mMapped;

        other.mBuffer = VK_NULL_HANDLE;
        other.mBufferMemory = VK_NULL_HANDLE;
        other.mBufferSize = 0;
        other.mMapped = nullptr;
    }
    return *this;
}

void BufferObject::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
) {
    cleanup(); // 确保清理旧资源

    // 创建缓冲区
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mContext.logicalDevice, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    // 分配内存
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mContext.logicalDevice, mBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(mContext.logicalDevice, &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS) {
        vkDestroyBuffer(mContext.logicalDevice, mBuffer, nullptr);
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(mContext.logicalDevice, mBuffer, mBufferMemory, 0);
    mBufferSize = size;
}

// 带暂存缓冲区的数据上传
template <typename DataType>
void BufferObject::uploadData(const DataType* data, VkDeviceSize dataSize, VkBufferUsageFlags finalUsage) {
    // 创建暂存缓冲区
    BufferObject stagingBuffer(mContext);
    stagingBuffer.createBuffer(
        dataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    // 映射并拷贝数据
    void* mapped;
    vkMapMemory(mContext.logicalDevice, stagingBuffer.mBufferMemory, 0, dataSize, 0, &mapped);
    memcpy(mapped, data, dataSize);
    vkUnmapMemory(mContext.logicalDevice, stagingBuffer.mBufferMemory);

    // 创建目标缓冲区
    createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | finalUsage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 执行拷贝命令
    copyBuffer(stagingBuffer.mBuffer, mBuffer, dataSize);
}

// 内存类型查找
uint32_t BufferObject::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mContext.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

// 缓冲区拷贝
void BufferObject::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mContext.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(mContext.logicalDevice, &allocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(mContext.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mContext.graphicsQueue);

    vkFreeCommandBuffers(mContext.logicalDevice, mContext.commandPool, 1, &cmdBuffer);
}
