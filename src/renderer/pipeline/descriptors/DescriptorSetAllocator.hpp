#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include <memory>
#include "descriptorPool.hpp"

class LogicalDevice;
class DescriptorLayoutManager;
class ResourceManager;

class DescriptorSetAllocator {
public:
    void createDescriptorPool(const LogicalDevice::Ptr& device,
        const DescriptorLayoutManager& layoutManager,
        uint32_t imageCount);

    void allocateDescriptorSets(const LogicalDevice::Ptr& device,
        const DescriptorLayoutManager& layoutManager,
        uint32_t imageCount);

    void updateDescriptorSets(const LogicalDevice::Ptr& device,
        const DescriptorLayoutManager& layoutManager,
        const ResourceManager& resourceManager,
        uint32_t imageCount);

    const std::map<uint32_t, std::vector<VkDescriptorSet>>& getDescriptorSets() const {
        return mDescriptorSets;
    }

    void cleanup();

private:
    DescriptorPool::Ptr mDescriptorPool;
    std::map<uint32_t, std::vector<VkDescriptorSet>> mDescriptorSets;
};