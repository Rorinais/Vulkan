#include "DescriptorSetAllocator.hpp"
#include "DescriptorLayoutManager.hpp"
#include "ResourceManager.hpp"
#include "DescriptorResource.hpp"
#include <stdexcept>
#include <vector>

void DescriptorSetAllocator::createDescriptorPool(
    const LogicalDevice::Ptr& device,
    const DescriptorLayoutManager& layoutManager,
    uint32_t imageCount)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    std::map<VkDescriptorType, uint32_t> typeCounts;

    for (const auto& [set, bindings] : layoutManager.getBindings()) {
        for (const auto& binding : bindings) {
            typeCounts[binding.layoutBinding.descriptorType] += imageCount;
        }
    }

    for (const auto& [type, count] : typeCounts) {
        poolSizes.push_back(VkDescriptorPoolSize{ type, count });
    }

    mDescriptorPool = DescriptorPool::create(
        device,
        poolSizes,
        static_cast<uint32_t>(layoutManager.getBindings().size() * imageCount)
    );
}

void DescriptorSetAllocator::allocateDescriptorSets(
    const LogicalDevice::Ptr& device,
    const DescriptorLayoutManager& layoutManager,
    uint32_t imageCount)
{
    for (const auto& [set, layout] : layoutManager.getLayouts()) {
        std::vector<VkDescriptorSetLayout> layouts(imageCount, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool->getHandle();
        allocInfo.descriptorSetCount = imageCount;
        allocInfo.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> descriptorSets(imageCount);
        if (vkAllocateDescriptorSets(device->getHandle(), &allocInfo,
            descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets");
        }
        mDescriptorSets[set] = std::move(descriptorSets);
    }
}

void DescriptorSetAllocator::updateDescriptorSets(
    const LogicalDevice::Ptr& device,
    const DescriptorLayoutManager& layoutManager,
    const ResourceManager& resourceManager,
    uint32_t imageCount)
{
    for (const auto& [set, bindings] : layoutManager.getBindings()) {
        for (uint32_t i = 0; i < imageCount; ++i) {
            VkDescriptorSet descriptorSet = mDescriptorSets.at(set)[i];
            std::vector<VkWriteDescriptorSet> writes;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;

            for (const auto& binding : bindings) {
                uint32_t bindingIndex = binding.layoutBinding.binding;
                auto resource = resourceManager.getResource(set, bindingIndex, i);

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptorSet;
                write.dstBinding = bindingIndex;
                write.dstArrayElement = 0;
                write.descriptorCount = 1;
                write.descriptorType = binding.layoutBinding.descriptorType;

                if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    auto* ubResource = dynamic_cast<UniformBufferResource*>(resource);
                    if (!ubResource) {
                        throw std::runtime_error("Resource type mismatch for uniform buffer");
                    }
                    const auto& bufferData = ubResource->getData();

                    bufferInfos.push_back(VkDescriptorBufferInfo{
                        bufferData.buffer,
                        0,
                        binding.dataSize
                        });

                    write.pBufferInfo = &bufferInfos.back();
                }
                else if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    auto* texResource = dynamic_cast<TextureResource*>(resource);
                    if (!texResource) {
                        throw std::runtime_error("Resource type mismatch for texture");
                    }
                    auto texture = texResource->getTexture();

                    imageInfos.push_back(VkDescriptorImageInfo{
                        texture->getSampler(),
                        texture->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        });

                    write.pImageInfo = &imageInfos.back();
                }
                writes.push_back(write);
            }

            vkUpdateDescriptorSets(device->getHandle(),
                static_cast<uint32_t>(writes.size()),
                writes.data(), 0, nullptr);
        }
    }
}

void DescriptorSetAllocator::cleanup() {
    mDescriptorSets.clear();
    mDescriptorPool.reset();
}