#pragma once
#include "DescriptorResource.hpp"
#include "DescriptorLayoutManager.hpp"
#include <map>
#include <vector>
#include <unordered_set>
#include <memory>

class ResourceManager {
public:
    ResourceManager(const LogicalDevice::Ptr& device,
        const CommandPool::Ptr& commandPool);
    ~ResourceManager();

    void createResources(const DescriptorLayoutManager& layoutManager,
        uint32_t imageCount);
    DescriptorResource* getResource(uint32_t set, uint32_t binding, uint32_t imageIndex) const;
    void cleanup();

private:
    LogicalDevice::Ptr mDevice;
    CommandPool::Ptr mCommandPool;
    std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorResource::Ptr>>> mResources;
    std::vector<DescriptorResource::Ptr> mAllResources;
};