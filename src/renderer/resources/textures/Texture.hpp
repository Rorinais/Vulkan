#pragma once
#include "../../../base.hpp"
#include "../../core/context/logicalDevice.hpp"
#include "../../core/commands/commandPool.hpp"
#include <stdexcept>

class Texture {
public:
    enum class Type {
        Color,
        Depth
    };

    using Ptr = std::shared_ptr<Texture>;
    static Ptr create(const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool,
        const char* imagePath) {
        return std::make_shared<Texture>(logicalDevice, commandPool, imagePath);
    }

    Texture(
        const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool, 
        const char* imagePath);

    static Ptr create(const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool,
        Type type,
        VkExtent2D extent) {
        return std::make_shared<Texture>(logicalDevice, commandPool, type, extent);
    }

    Texture(
        const LogicalDevice::Ptr& logicalDevice, 
        const CommandPool::Ptr& commandPool, 
        Type type, 
        VkExtent2D extent);

    ~Texture();

    static VkFormat findSupportedDepthFormat(VkPhysicalDevice physicalDevice);
    static bool hasStencilComponent(VkFormat format); 

    VkImageView getImageView() const { return mImageView; }
    VkSampler getSampler() const { return mSampler; }

private:
    LogicalDevice::Ptr mLogicalDevice;
    CommandPool::Ptr mCommandPool;

    VkImage mImage = VK_NULL_HANDLE;
    VkImageView mImageView = VK_NULL_HANDLE;
    VkSampler mSampler = VK_NULL_HANDLE;
    VkDeviceMemory mMemory = VK_NULL_HANDLE;

    Type mType = Type::Color;
    VkFormat mFormat = VK_FORMAT_UNDEFINED;

    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;
    std::vector<uint8_t> pixels;

private:
    void loadTexture(const char* imagePath);
    void createImage(VkFormat format, VkExtent2D extent, VkImageUsageFlags usage, VkImageTiling tiling);
    void allocateMemory(VkMemoryPropertyFlags properties);
    void createImageView();
    void createSampler(const VkSamplerCreateInfo& samplerInfo);
    void uploadData(const void* data, size_t dataSize, VkExtent2D extent);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, VkExtent2D extent);
    uint32_t findMemoryType(uint32_t typeFilter,VkMemoryPropertyFlags properties) const;
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};
