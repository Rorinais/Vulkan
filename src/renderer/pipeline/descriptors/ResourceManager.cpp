#include "ResourceManager.hpp"

ResourceManager::ResourceManager(const LogicalDevice::Ptr& device,
    const CommandPool::Ptr& commandPool)
    : mDevice(device), mCommandPool(commandPool) {
}

ResourceManager::~ResourceManager() {
    cleanup();
}

void ResourceManager::createResources(const DescriptorLayoutManager& layoutManager,
    uint32_t imageCount) {
    for (const auto& [set, bindings] : layoutManager.getBindings()) {
        for (const auto& binding : bindings) {
            uint32_t bindingIndex = binding.layoutBinding.binding;

            if (binding.layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (uint32_t i = 0; i < imageCount; ++i) {
                    auto resource = std::make_shared<UniformBufferResource>(
                        mDevice, mCommandPool, binding.dataSize
                    );
                    mResources[set][bindingIndex].push_back(resource);
                    mAllResources.push_back(resource);
                }
            }
            else if (binding.layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                auto resource = std::make_shared<TextureResource>(
                    mDevice, mCommandPool, binding.texturePath.c_str()
                );
                mAllResources.push_back(resource);

                std::vector<DescriptorResource::Ptr> resources(imageCount, resource);
                mResources[set][bindingIndex] = resources;
            }
        }
    }
}

DescriptorResource* ResourceManager::getResource(uint32_t set, uint32_t binding, uint32_t imageIndex) const {
    return mResources.at(set).at(binding).at(imageIndex).get();
}

void ResourceManager::cleanup() {
    mResources.clear();
    mAllResources.clear();
}