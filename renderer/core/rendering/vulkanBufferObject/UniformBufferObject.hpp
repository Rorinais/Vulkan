#pragma once
#include <vector>
#include <map>
#include <stdexcept>
#include <unordered_set>
#include "../../base.hpp"
#include"Texture.hpp"


class UniformBufferManager {
private:
    struct BindingInfo {
        VkDescriptorSetLayoutBinding layoutBinding{};
        size_t dataSize = 0;
        Texture* texture = nullptr;
    };

    struct DescriptorResource {
        enum class Type { UniformBuffer, CombinedImageSampler };
        Type mType;
        VulkanContext mContext;

        union {
            struct {
                VkBuffer buffer = VK_NULL_HANDLE;
                VkDeviceMemory memory = VK_NULL_HANDLE;
                void* mapped = nullptr;
                size_t size = 0;
            } uniformBuffer;

            struct {
                Texture* imageBuffer;
            }image;

        };

        DescriptorResource(VulkanContext& context, Type type, const BindingInfo& bindingInfo)
            : mContext(context), mType(type) {
            if (mType == Type::UniformBuffer) {
                createBuffer(bindingInfo.dataSize,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }
            else if (mType == Type::CombinedImageSampler) {
                mType = Type::CombinedImageSampler;
                image.imageBuffer = bindingInfo.texture;
            }
        }

        ~DescriptorResource() {
            if (mType == Type::UniformBuffer) {
                cleanupUniformBuffer();
            }
            else if (mType == Type::CombinedImageSampler) {
                delete image.imageBuffer;
            }
        }

    private:
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceProperties physDevProps{};
            vkGetPhysicalDeviceProperties(mContext.physicalDevice, &physDevProps);
            VkDeviceSize alignment = physDevProps.limits.minUniformBufferOffsetAlignment;
            if (alignment > 0)
                size = (size + alignment - 1) & ~(alignment - 1);

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(mContext.logicalDevice, &bufferInfo, nullptr,
                &uniformBuffer.buffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create uniform buffer");

            VkMemoryRequirements memReqs{};
            vkGetBufferMemoryRequirements(mContext.logicalDevice,
                uniformBuffer.buffer, &memReqs);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits,
                properties);

            if (vkAllocateMemory(mContext.logicalDevice, &allocInfo, nullptr,
                &uniformBuffer.memory) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate uniform buffer memory");

            vkBindBufferMemory(mContext.logicalDevice, uniformBuffer.buffer,
                uniformBuffer.memory, 0);

            if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                vkMapMemory(mContext.logicalDevice, uniformBuffer.memory, 0,
                    size, 0, &uniformBuffer.mapped);
            }

            uniformBuffer.size = size;
        }
        uint32_t findMemoryType(uint32_t typeFilter,VkMemoryPropertyFlags properties) const {
            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(mContext.physicalDevice, &memProps);

            for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) &&
                    (memProps.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }
            throw std::runtime_error("Failed to find suitable memory type");
        }
        void cleanupUniformBuffer() {
            if (uniformBuffer.mapped) {
                vkUnmapMemory(mContext.logicalDevice, uniformBuffer.memory);
                uniformBuffer.mapped = nullptr;
            }
            if (uniformBuffer.buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(mContext.logicalDevice, uniformBuffer.buffer, nullptr);
                uniformBuffer.buffer = VK_NULL_HANDLE;
            }
            if (uniformBuffer.memory != VK_NULL_HANDLE) {
                vkFreeMemory(mContext.logicalDevice, uniformBuffer.memory, nullptr);
                uniformBuffer.memory = VK_NULL_HANDLE;
            }
            uniformBuffer.size = 0;
        }
    };

    VulkanContext& mContext;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    std::map<uint32_t, std::vector<BindingInfo>> mSetBindings;
    std::map<uint32_t, VkDescriptorSetLayout> mDescriptorSetLayouts;
    std::map<uint32_t, std::vector<VkDescriptorSet>> mDescriptorSets;
    std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorResource*>>> mUniformBuffers;

public:
    UniformBufferManager(VulkanContext& context) : mContext(context) {}
    ~UniformBufferManager() {
        cleanupResources();
    }

    void createDescriptorResources(const uint32_t imageCount) {
        createDescriptorSetLayouts();
        createDescriptorPool(imageCount);
        createUniformBuffers(imageCount);
        createDescriptorSets(imageCount);
    }

    void cleanupResources() {
        std::unordered_set<DescriptorResource*> uniqueResources;
        for (auto& setEntry : mUniformBuffers) {
            for (auto& bindingEntry : setEntry.second) {
                for (auto resource : bindingEntry.second) {
                    uniqueResources.insert(resource);
                }
                bindingEntry.second.clear();
            }
        }
        for (auto resource : uniqueResources) {
            delete resource;
        }
        mUniformBuffers.clear();

        for (auto& [set, layout] : mDescriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(mContext.logicalDevice, layout, nullptr);
        }
        mDescriptorSetLayouts.clear();

        if (mDescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(mContext.logicalDevice, mDescriptorPool, nullptr);
            mDescriptorPool = VK_NULL_HANDLE;
        }
        mDescriptorSets.clear();
    }

    template<class T>
    void addUniformBinding(uint32_t set, uint32_t binding,
        VkShaderStageFlags stageFlags) {
        BindingInfo info{};
        info.layoutBinding.binding = binding;
        info.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        info.layoutBinding.descriptorCount = 1;
        info.layoutBinding.stageFlags = stageFlags;
        info.dataSize = sizeof(T);
        mSetBindings[set].push_back(info);
    }

    void addTextureBinding(uint32_t set, uint32_t binding,VkShaderStageFlags stageFlags, const char* imagePath) {
        BindingInfo info{};
        info.layoutBinding.binding = binding;
        info.layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        info.layoutBinding.descriptorCount = 1;
        info.layoutBinding.stageFlags = stageFlags;
        info.texture = new Texture(mContext,imagePath);

        mSetBindings[set].push_back(info);
    }

    template<typename T>
    void updateUniformData(uint32_t set, uint32_t binding, uint32_t imageIndex,
        const T& data) {
        auto& ubArray = mUniformBuffers.at(set).at(binding);
        if (imageIndex >= ubArray.size())
            throw std::out_of_range("Image index out of range");
        if (ubArray[imageIndex]->mType != DescriptorResource::Type::UniformBuffer)
            throw std::runtime_error("Binding is not a uniform buffer");
        memcpy(ubArray[imageIndex]->uniformBuffer.mapped, &data, sizeof(T));
    }

    std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() const {
        std::vector<VkDescriptorSetLayout> layouts;
        for (const auto& [set, layout] : mDescriptorSetLayouts) {
            layouts.push_back(layout);
        }
        return layouts;
    }

    std::vector<std::vector<VkDescriptorSet>> getDescriptorSetss() const {
        std::vector<std::vector<VkDescriptorSet>> result;

        uint32_t maxSet = 0;
        for (const auto& [set, _] : mDescriptorSets) {
            if (set > maxSet) maxSet = set;
        }

        for (uint32_t set = 0; set <= maxSet; ++set) {
            if (mDescriptorSets.count(set)) {
                result.push_back(mDescriptorSets.at(set));
            }
        }
        return result;
    }

    std::vector<uint32_t> getDescriptorSetNumbers() const {
        std::vector<uint32_t> sets;
        for (const auto& [set, _] : mDescriptorSetLayouts) {
            sets.push_back(set);
        }
        std::sort(sets.begin(), sets.end());
        return sets;
    }

private:
    void createDescriptorSetLayouts() {
        for (auto& [set, bindings] : mSetBindings) {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (const auto& binding : bindings) {
                layoutBindings.push_back(binding.layoutBinding);
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();

            if (vkCreateDescriptorSetLayout(mContext.logicalDevice, &layoutInfo,
                nullptr, &mDescriptorSetLayouts[set]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create descriptor set layout");
        }
    }

    void createDescriptorPool(uint32_t imageCount) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        std::map<VkDescriptorType, uint32_t> typeCounts;

        for (const auto& [set, bindings] : mSetBindings) {
            for (const auto& binding : bindings) {
                typeCounts[binding.layoutBinding.descriptorType] += imageCount;
            }
        }

        for (const auto& [type, count] : typeCounts) {
            poolSizes.push_back({type, count});
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(mSetBindings.size() * imageCount);

        if (vkCreateDescriptorPool(mContext.logicalDevice, &poolInfo, nullptr,&mDescriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create descriptor pool");
    }

    void createDescriptorSets(uint32_t imageCount) {
        for (const auto& [set, bindings] : mSetBindings) {
            std::vector<VkDescriptorSetLayout> layouts(imageCount, mDescriptorSetLayouts[set]);
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = mDescriptorPool;
            allocInfo.descriptorSetCount = imageCount;
            allocInfo.pSetLayouts = layouts.data();

            std::vector<VkDescriptorSet> descriptorSets(imageCount);
            if (vkAllocateDescriptorSets(mContext.logicalDevice, &allocInfo,
                descriptorSets.data()) != VK_SUCCESS)
                throw std::runtime_error("Failed to allocate descriptor sets");
            mDescriptorSets[set] = descriptorSets;

            // 更新描述符集写入
            for (uint32_t i = 0; i < imageCount; ++i) {
                std::vector<VkWriteDescriptorSet> writes;
                std::vector<VkDescriptorBufferInfo> bufferInfos;
                std::vector<VkDescriptorImageInfo> imageInfos;

                for (const auto& binding : bindings) {
                    uint32_t bindingIndex = binding.layoutBinding.binding;
                    auto& resource = mUniformBuffers[set][bindingIndex][i];

                    if (binding.layoutBinding.descriptorType ==
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                        VkDescriptorBufferInfo bufferInfo{};
                        bufferInfo.buffer = resource->uniformBuffer.buffer;
                        bufferInfo.offset = 0;
                        bufferInfo.range = binding.dataSize;
                        bufferInfos.push_back(bufferInfo);

                        writes.push_back(VkWriteDescriptorSet{
                            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            nullptr,
                            descriptorSets[i],
                            bindingIndex,
                            0,
                            1,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            nullptr,
                            &bufferInfos.back(),
                            nullptr
                            });
                    }
                    else if (binding.layoutBinding.descriptorType ==
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfo.imageView = resource->image.imageBuffer->imageView();
                        imageInfo.sampler = resource->image.imageBuffer->sampler();
                        imageInfos.push_back(imageInfo);

                        writes.push_back(VkWriteDescriptorSet{
                            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            nullptr,
                            descriptorSets[i],
                            bindingIndex,
                            0,
                            1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            &imageInfos.back(),
                            nullptr,
                            nullptr
                            });
                    }
                }

                if (!writes.empty()) {
                    vkUpdateDescriptorSets(mContext.logicalDevice,
                        static_cast<uint32_t>(writes.size()),
                        writes.data(), 0, nullptr);
                }
            }
        }
    }

    void createUniformBuffers(uint32_t imageCount) {
        for (const auto& [set, bindings] : mSetBindings) {
            for (const auto& binding : bindings) {
                uint32_t bindingIndex = binding.layoutBinding.binding;
                auto& resources = mUniformBuffers[set][bindingIndex];

                if (resources.empty()) {
                    if (binding.layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                        for (uint32_t i = 0; i < imageCount; ++i) {
                            resources.push_back(new DescriptorResource(mContext,
                                DescriptorResource::Type::UniformBuffer, binding));
                        }
                    }
                    else if (binding.layoutBinding.descriptorType ==
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                        DescriptorResource* texResource = new DescriptorResource(
                            mContext, DescriptorResource::Type::CombinedImageSampler, binding);
                        resources.resize(imageCount, texResource); 
                    }
                }
            }
        }
    }
};