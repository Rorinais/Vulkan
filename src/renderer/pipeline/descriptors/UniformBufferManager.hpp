#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "../../../base.hpp"
#include "DescriptorLayoutManager.hpp"
#include "ResourceManager.hpp"
#include "DescriptorSetAllocator.hpp"

class UniformBufferManager {
public:
    using Ptr = std::shared_ptr<UniformBufferManager>;

    UniformBufferManager(const LogicalDevice::Ptr& logicalDevice,
        const CommandPool::Ptr& commandPool);
    ~UniformBufferManager();

    void createDescriptorResources(uint32_t imageCount);
    void cleanupResources();

    template<class T>
    void addUniformBinding(uint32_t set, uint32_t binding,
        VkShaderStageFlags stageFlags) {
        BindingInfo info{};
        info.layoutBinding.binding = binding;
        info.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        info.layoutBinding.descriptorCount = 1;
        info.layoutBinding.stageFlags = stageFlags;
        info.dataSize = sizeof(T);
        mLayoutManager->addBinding(set, info);
    }

    void addTextureBinding(uint32_t set, uint32_t binding,
        VkShaderStageFlags stageFlags,
        const char* imagePath) {
        BindingInfo info{};
        info.layoutBinding.binding = binding;
        info.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        info.layoutBinding.descriptorCount = 1;
        info.layoutBinding.stageFlags = stageFlags;
        info.texturePath = imagePath;
        mLayoutManager->addBinding(set, info);
    }

    template<typename T>
    void updateUniformData(uint32_t set, uint32_t binding,
        uint32_t imageIndex, const T& data) {
        auto resource = mResourceManager->getResource(set, binding, imageIndex);
        if (resource->getType() != DescriptorResource::Type::UniformBuffer) {
            throw std::runtime_error("Binding is not a uniform buffer");
        }

        auto* ubResource = static_cast<UniformBufferResource*>(resource);
        ubResource->update(&data, sizeof(T));
    }

    std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() const;
    std::vector<VkDescriptorSet> getDescriptorSetsForFrame(uint32_t frameIndex) const;

private:
    LogicalDevice::Ptr mLogicalDevice;
    CommandPool::Ptr mCommandPool;
    std::unique_ptr<DescriptorLayoutManager> mLayoutManager;
    std::unique_ptr<ResourceManager> mResourceManager;
    std::unique_ptr<DescriptorSetAllocator> mSetAllocator;
};