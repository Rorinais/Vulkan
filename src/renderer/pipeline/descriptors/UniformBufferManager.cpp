#include "UniformBufferManager.hpp"

UniformBufferManager::UniformBufferManager(const LogicalDevice::Ptr& logicalDevice,
    const CommandPool::Ptr& commandPool)
    : mLogicalDevice(logicalDevice),
    mCommandPool(commandPool),
    mLayoutManager(std::make_unique<DescriptorLayoutManager>()),
    mResourceManager(std::make_unique<ResourceManager>(logicalDevice, commandPool)),
    mSetAllocator(std::make_unique<DescriptorSetAllocator>()) {
}

UniformBufferManager::~UniformBufferManager() {
    cleanupResources();
}

void UniformBufferManager::createDescriptorResources(uint32_t imageCount) {
    mLayoutManager->createLayouts(mLogicalDevice);
    mResourceManager->createResources(*mLayoutManager, imageCount);
    mSetAllocator->createDescriptorPool(mLogicalDevice, *mLayoutManager, imageCount);
    mSetAllocator->allocateDescriptorSets(mLogicalDevice, *mLayoutManager, imageCount);
    mSetAllocator->updateDescriptorSets(mLogicalDevice, *mLayoutManager, *mResourceManager, imageCount);
}

void UniformBufferManager::cleanupResources() {
    mSetAllocator->cleanup();
    mResourceManager->cleanup();
    mLayoutManager->cleanup(mLogicalDevice);
}

std::vector<VkDescriptorSetLayout> UniformBufferManager::getDescriptorSetLayouts() const {
    std::vector<VkDescriptorSetLayout> layouts;
    for (const auto& [set, layout] : mLayoutManager->getLayouts()) {
        layouts.push_back(layout);
    }
    return layouts;
}

std::vector<VkDescriptorSet> UniformBufferManager::getDescriptorSetsForFrame(uint32_t frameIndex) const {
    std::vector<VkDescriptorSet> frameSets;
    for (const auto& [set, descriptorSets] : mSetAllocator->getDescriptorSets()) {
        if (frameIndex < descriptorSets.size()) {
            frameSets.push_back(descriptorSets[frameIndex]);
        }
    }
    return frameSets;
}