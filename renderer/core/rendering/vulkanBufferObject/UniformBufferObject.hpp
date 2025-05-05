#pragma once
#include <vector>
#include <map>
#include <stdexcept>
#include <unordered_set>
#include "./BufferObject.hpp"

class UniformBufferManager {
private:
    struct BindingInfo {
        VkDescriptorSetLayoutBinding layoutBinding;
        size_t dataSize;
    };

    struct UniformBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void* mapped = nullptr;
        size_t size = 0;
    };

    VulkanContext& mContext;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

    std::map<uint32_t, std::vector<BindingInfo>> mSetBindings;
    std::map<uint32_t, VkDescriptorSetLayout> mDescriptorSetLayouts;
    std::map<uint32_t, std::vector<VkDescriptorSet>> mDescriptorSets;
    std::map<uint32_t, std::map<uint32_t, std::vector<UniformBuffer>>> mUniformBuffers;
public:
    UniformBufferManager(VulkanContext& context) : mContext(context) {}
    ~UniformBufferManager() {
        cleanupResources();
    }

    template<class T>
    void addUniformBinding(uint32_t set, uint32_t binding, VkShaderStageFlags stageFlags) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = stageFlags;

        mSetBindings[set].push_back({ layoutBinding, sizeof(T) });
    }

    void createDescriptorResources(const uint32_t imageCount) {
        createDescriptorSetLayouts();
        createDescriptorPool(imageCount);
        createUniformBuffers(imageCount);
        createDescriptorSets(imageCount);
    }

    template<typename T>
    void updateUniformData(uint32_t set, uint32_t binding, uint32_t imageIndex, const T& data) {
        try {
            auto& ubArray = mUniformBuffers.at(set).at(binding);
            if (imageIndex >= ubArray.size()) {
                throw std::out_of_range("Image index out of range");
            }
            memcpy(ubArray[imageIndex].mapped, &data, sizeof(T));
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to update uniform data: ") + e.what());
        }
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

        // 确保按set编号顺序返回
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

void cleanupResources() {
    if (mDescriptorSetLayouts.empty() && mDescriptorPool == VK_NULL_HANDLE && mUniformBuffers.empty()) {
        return;
    }
    for (auto& [set, layout] : mDescriptorSetLayouts) {
        if (layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(mContext.logicalDevice, layout, nullptr);
            layout = VK_NULL_HANDLE; 
        }
    }
    mDescriptorSetLayouts.clear();

    if (mDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(mContext.logicalDevice, mDescriptorPool, nullptr);
        mDescriptorPool = VK_NULL_HANDLE;
    }

    for (auto& setEntry : mUniformBuffers) {
        auto& bindings = setEntry.second;
        for (auto& bindingEntry : bindings) {
            auto& buffers = bindingEntry.second;
            for (auto& buffer : buffers) {
                if (buffer.mapped) {
                    vkUnmapMemory(mContext.logicalDevice, buffer.memory);
                    buffer.mapped = nullptr;
                }
                if (buffer.buffer != VK_NULL_HANDLE) {
                    vkDestroyBuffer(mContext.logicalDevice, buffer.buffer, nullptr);
                    buffer.buffer = VK_NULL_HANDLE;
                }
                if (buffer.memory != VK_NULL_HANDLE) {
                    vkFreeMemory(mContext.logicalDevice, buffer.memory, nullptr);
                    buffer.memory = VK_NULL_HANDLE;
                }
            }
            buffers.clear();
        }
        bindings.clear();
    }
    mUniformBuffers.clear();
}

    void recreateResources(uint32_t newImageCount) {
        cleanupResources();
        createDescriptorResources(newImageCount);
    }

private:
    void createDescriptorSetLayouts() {
        for (auto& [set, bindings] : mSetBindings) {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (const auto& bindingInfo : bindings) {
                layoutBindings.push_back(bindingInfo.layoutBinding);
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();

            if (vkCreateDescriptorSetLayout(
                mContext.logicalDevice,
                &layoutInfo,
                nullptr,
                &mDescriptorSetLayouts[set]
            ) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }
    }

    void createDescriptorPool(const uint32_t imageCount) {
        std::unordered_map<VkDescriptorType, uint32_t> typeCounts;

        for (const auto& [set, bindings] : mSetBindings) {
            for (const auto& binding : bindings) {
                typeCounts[binding.layoutBinding.descriptorType] += imageCount;
            }
        }

        std::vector<VkDescriptorPoolSize> poolSizes;
        for (const auto& [type, count] : typeCounts) {
            poolSizes.push_back({ type, count });
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(mSetBindings.size() * imageCount);

        if (vkCreateDescriptorPool(
            mContext.logicalDevice,
            &poolInfo,
            nullptr,
            &mDescriptorPool
        ) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void createDescriptorSets(const uint32_t imageCount) {
        for (const auto& [set, bindings] : mSetBindings) {
            std::vector<VkDescriptorSetLayout> layouts(imageCount, mDescriptorSetLayouts[set]);

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = mDescriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
            allocInfo.pSetLayouts = layouts.data();

            std::vector<VkDescriptorSet> descriptorSets(imageCount);
            if (vkAllocateDescriptorSets(mContext.logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }
            mDescriptorSets[set] = descriptorSets;

            for (size_t i = 0; i < imageCount; ++i) {
                std::vector<VkWriteDescriptorSet> writes;
                std::vector<VkDescriptorBufferInfo> bufferInfos;
                // 预留空间以避免重新分配
                bufferInfos.reserve(bindings.size());
                writes.reserve(bindings.size());

                for (const auto& binding : bindings) {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = mUniformBuffers[set][binding.layoutBinding.binding][i].buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range = binding.dataSize;
                    bufferInfos.push_back(bufferInfo);

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptorSets[i];
                    write.dstBinding = binding.layoutBinding.binding;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    write.descriptorCount = 1;
                    write.pBufferInfo = &bufferInfos.back();
                    writes.push_back(write);
                }

                vkUpdateDescriptorSets(
                    mContext.logicalDevice,
                    static_cast<uint32_t>(writes.size()),
                    writes.data(),
                    0, nullptr
                );
            }
        }
    }

    void createUniformBuffers(const uint32_t imageCount) {
        for (const auto& [set, bindings] : mSetBindings) {
            for (const auto& binding : bindings) {
                const uint32_t bindingIndex = binding.layoutBinding.binding;

                // 确保每个binding的缓冲数组初始化
                if (mUniformBuffers[set].find(bindingIndex) == mUniformBuffers[set].end()) {
                    mUniformBuffers[set][bindingIndex] = std::vector<UniformBuffer>();
                }

                for (size_t i = 0; i < imageCount; ++i) {
                    UniformBuffer ub{};
                    createBuffer(
                        binding.dataSize,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        ub.buffer,
                        ub.memory
                    );
                    vkMapMemory(
                        mContext.logicalDevice,
                        ub.memory,
                        0,
                        binding.dataSize,
                        0,
                        &ub.mapped
                    );
                    mUniformBuffers[set][bindingIndex].push_back(ub);
                }
            }
        }
    }

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory
    ) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(mContext.physicalDevice, &props);
        VkDeviceSize alignment = props.limits.minUniformBufferOffsetAlignment;

        // 确保size是alignment的整数倍
        if (alignment > 0) {
            size = (size + alignment - 1) & ~(alignment - 1);
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(mContext.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(mContext.logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mContext.logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            vkDestroyBuffer(mContext.logicalDevice, buffer, nullptr);
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(mContext.logicalDevice, buffer, bufferMemory, 0);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
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

    void validateSetBinding(uint32_t set, uint32_t binding) const {
        if (mUniformBuffers.find(set) == mUniformBuffers.end()) {
            throw std::invalid_argument("Descriptor set not exists");
        }
        if (mUniformBuffers.at(set).find(binding) == mUniformBuffers.at(set).end()) {
            throw std::invalid_argument("Binding not exists in set");
        }
    }
};