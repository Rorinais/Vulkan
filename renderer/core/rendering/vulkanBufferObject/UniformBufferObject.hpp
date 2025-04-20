#pragma once
#include "BufferObject.hpp"
#include "../../base.hpp"
#include <glm/gtx/string_cast.hpp>
#include <map>
#include <stdexcept>

#define MAX_DESCRIPTOR_BINDINGS 16

class UniformBufferBase : public BufferObject {
public:
	using BufferObject::BufferObject;

	virtual void updateData(const void* data, size_t size) = 0;
	virtual VkDescriptorBufferInfo getDescriptorInfo() const = 0;
	virtual ~UniformBufferBase() = default;
};

template <typename T>
class TypedUniformBuffer : public UniformBufferBase {
public:
	TypedUniformBuffer(const VulkanContext& context)
		: UniformBufferBase(context) {
	}

	void initialize() {
		createBuffer(
			sizeof(T),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		vkMapMemory(mContext.logicalDevice, mBufferMemory, 0, sizeof(T), 0, &mMapped);
	}

	void updateData(const void* data, size_t size) override {
		if (size != sizeof(T)) {
			throw std::runtime_error("UBO data size mismatch!");
		}
		memcpy(mMapped, data, sizeof(T));
	}

	VkDescriptorBufferInfo getDescriptorInfo() const override {
		return { mBuffer, 0, sizeof(T) };
	}

	void cleanup() noexcept override {
		if (mMapped) {
			vkUnmapMemory(mContext.logicalDevice, mBufferMemory);
			mMapped = nullptr;
		}
		BufferObject::cleanup();
	}

private:
	void* mMapped = nullptr;
};

struct UBOLayoutInfo {
	uint32_t binding;
	VkShaderStageFlags stageFlags;
	size_t dataSize;
};

//class MultiUBOManager {
//public:
//	MultiUBOManager(const VulkanContext& context, size_t swapChainImageCount)
//		: mContext(context), mSwapChainCount(swapChainImageCount) {
//	}
//
//	template <typename T>
//	void addUBOBinding(uint32_t bindingPoint, VkShaderStageFlags stages) {
//		if (mUBOMap.count(bindingPoint) > 0) {
//			throw std::runtime_error("Binding point already in use!");
//		}
//
//		if (bindingPoint >= MAX_DESCRIPTOR_BINDINGS) {
//			throw std::runtime_error("Exceeded maximum binding points!");
//		}
//
//		try {
//			UBOLayoutInfo info{ bindingPoint, stages, sizeof(T) };
//			std::vector<std::unique_ptr<UniformBufferBase>> buffers;
//
//			for (size_t i = 0; i < mSwapChainCount; ++i) {
//				auto buffer = std::make_unique<TypedUniformBuffer<T>>(mContext);
//				buffer->initialize();
//				buffers.emplace_back(std::move(buffer));
//			}
//
//			mUBOMap.emplace(bindingPoint, UBOInfo{ info, std::move(buffers) });
//		}
//		catch (const std::exception& e) {
//			throw std::runtime_error("Failed to create UBO binding: " + std::string(e.what()));
//		}
//	}
//
//	void createDescriptorLayout() {
//		std::vector<VkDescriptorSetLayoutBinding> bindings;
//
//		// 显式按绑定点排序
//		std::vector<uint32_t> sortedBindings;
//		for (const auto& pair : mUBOMap) {
//			sortedBindings.push_back(pair.first);
//		}
//		std::sort(sortedBindings.begin(), sortedBindings.end());
//
//		for (auto binding : sortedBindings) {
//			const auto& info = mUBOMap.at(binding);
//			VkDescriptorSetLayoutBinding layoutBinding{};
//			layoutBinding.binding = info.layoutInfo.binding;
//			layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//			layoutBinding.descriptorCount = 1;
//			layoutBinding.stageFlags = info.layoutInfo.stageFlags;
//			bindings.push_back(layoutBinding);
//		}
//
//		VkDescriptorSetLayoutCreateInfo layoutInfo{};
//		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//		layoutInfo.pBindings = bindings.data();
//
//		if (vkCreateDescriptorSetLayout(mContext.logicalDevice, &layoutInfo,
//			nullptr, &mDescriptorLayout) != VK_SUCCESS) {
//			throw std::runtime_error("Failed to create descriptor set layout!");
//		}
//	}
//
//	void createDescriptorPool() {
//		std::vector<VkDescriptorPoolSize> poolSizes;
//		poolSizes.reserve(mUBOMap.size());
//
//		// 每个UBO类型需要单独计数
//		for (const auto& [binding, info] : mUBOMap) {
//			poolSizes.push_back({
//				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//				static_cast<uint32_t>(mSwapChainCount)
//				});
//		}
//
//		VkDescriptorPoolCreateInfo poolInfo{};
//		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//		poolInfo.pPoolSizes = poolSizes.data();
//		poolInfo.maxSets = static_cast<uint32_t>(mSwapChainCount);
//		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//
//		if (vkCreateDescriptorPool(mContext.logicalDevice, &poolInfo,
//			nullptr, &mDescriptorPool) != VK_SUCCESS) {
//			throw std::runtime_error("Failed to create descriptor pool!");
//		}
//	}
//
//	void createDescriptorSets() {
//		std::vector<VkDescriptorSetLayout> layouts(mSwapChainCount, mDescriptorLayout);
//
//		VkDescriptorSetAllocateInfo allocInfo{};
//		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//		allocInfo.descriptorPool = mDescriptorPool;
//		allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChainCount);
//		allocInfo.pSetLayouts = layouts.data();
//
//		mDescriptorSets.resize(mSwapChainCount);
//		if (vkAllocateDescriptorSets(mContext.logicalDevice, &allocInfo,
//			mDescriptorSets.data()) != VK_SUCCESS) {
//			throw std::runtime_error("Failed to allocate descriptor sets!");
//		}
//
//		for (size_t i = 0; i < mSwapChainCount; ++i) {
//			std::vector<VkWriteDescriptorSet> writes;
//
//			// 显式按绑定顺序排序
//			std::vector<uint32_t> sortedBindings;
//			for (const auto& pair : mUBOMap) {
//				sortedBindings.push_back(pair.first);
//			}
//			std::sort(sortedBindings.begin(), sortedBindings.end());
//
//			for (auto binding : sortedBindings) {
//				auto& info = mUBOMap.at(binding);
//				VkDescriptorBufferInfo bufferInfo = info.buffers[i]->getDescriptorInfo();
//
//				VkWriteDescriptorSet write{};
//				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//				write.dstSet = mDescriptorSets[i];
//				write.dstBinding = info.layoutInfo.binding;
//				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//				write.descriptorCount = 1;
//				write.pBufferInfo = &bufferInfo;
//
//				writes.push_back(write);
//			}
//
//			vkUpdateDescriptorSets(mContext.logicalDevice,
//				static_cast<uint32_t>(writes.size()),
//				writes.data(),
//				0, nullptr);
//		}
//	}
//
//	template <typename T>
//	void updateUBOData(uint32_t bindingPoint, uint32_t imageIndex, const T& data) {
//		auto& entry = mUBOMap.at(bindingPoint);
//		if (sizeof(T) != entry.layoutInfo.dataSize) {
//			throw std::runtime_error("UBO data size mismatch!");
//		}
//
//		auto* buffer = dynamic_cast<TypedUniformBuffer<T>*>(entry.buffers[imageIndex].get());
//		if (!buffer) {
//			throw std::runtime_error("Invalid UBO type!");
//		}
//
//		buffer->updateData(&data, sizeof(T));
//	}
//
//	VkDescriptorSetLayout& getDescriptorLayout() { return mDescriptorLayout; }
//	VkDescriptorSet& getDescriptorSet(uint32_t imageIndex) { return mDescriptorSets[imageIndex]; }
//	const std::vector<VkDescriptorSet>& getDescriptorSets() const { return mDescriptorSets; }
//
//
//	void cleanup() {
//		vkDestroyDescriptorPool(mContext.logicalDevice, mDescriptorPool, nullptr);
//		vkDestroyDescriptorSetLayout(mContext.logicalDevice, mDescriptorLayout, nullptr);
//		for (auto& [binding, info] : mUBOMap) {
//			for (auto& buffer : info.buffers) {
//				buffer->cleanup();
//			}
//		}
//		mUBOMap.clear();
//	}
//
//private:
//	struct UBOInfo {
//		UBOLayoutInfo layoutInfo;
//		std::vector<std::unique_ptr<UniformBufferBase>> buffers;
//
//		UBOInfo(UBOLayoutInfo info, std::vector<std::unique_ptr<UniformBufferBase>>&& bufs)
//			: layoutInfo(std::move(info)),
//			buffers(std::move(bufs)) {
//		}
//	};
//
//	VulkanContext mContext;
//	size_t mSwapChainCount;
//	std::unordered_map<uint32_t, UBOInfo> mUBOMap;
//	VkDescriptorSetLayout mDescriptorLayout = VK_NULL_HANDLE;
//	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
//	std::vector<VkDescriptorSet> mDescriptorSets;
//};

class MultiDescriptorManager {
public:
    MultiDescriptorManager(const VulkanContext& context, size_t swapChainImageCount)
        : mContext(context), mSwapChainCount(swapChainImageCount) {
    }

    // 添加绑定信息时需要指定set和binding
    template <typename T>
    void addUBOBinding(uint32_t set, uint32_t binding, VkShaderStageFlags stages) {
        if (mSetMap[set].bindings.count(binding) > 0) {
            throw std::runtime_error("Binding point already in use in this set!");
        }

        try {
            SetInfo& setInfo = mSetMap[set];
            UBOInfo info{ sizeof(T), stages };

            // 创建缓冲
            std::vector<std::unique_ptr<UniformBufferBase>> buffers;
            for (size_t i = 0; i < mSwapChainCount; ++i) {
                auto buffer = std::make_unique<TypedUniformBuffer<T>>(mContext);
                buffer->initialize();
                buffers.emplace_back(std::move(buffer));
            }

            setInfo.bindings.emplace(binding, BindingInfo{ info, std::move(buffers) });
        }
        catch (...) {
            throw std::runtime_error("Failed to create UBO binding");
        }
    }

    void createDescriptorLayouts() {
        mAllLayouts.clear();

        // 按set索引升序处理
        for (auto& [setIndex, setInfo] : mSetMap) {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

            // 按binding顺序处理
            std::vector<uint32_t> sortedBindings;
            for (const auto& [binding, _] : setInfo.bindings) {
                sortedBindings.push_back(binding);
            }
            std::sort(sortedBindings.begin(), sortedBindings.end());

            // 收集当前set的所有binding
            for (auto binding : sortedBindings) {
                const auto& info = setInfo.bindings.at(binding);

                VkDescriptorSetLayoutBinding layoutBinding{};
                layoutBinding.binding = binding;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                layoutBinding.descriptorCount = 1;
                layoutBinding.stageFlags = info.uboInfo.stageFlags;
                layoutBindings.push_back(layoutBinding);
            }

            // 为当前set创建包含所有binding的布局
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();

            VkDescriptorSetLayout layout;
            if (vkCreateDescriptorSetLayout(mContext.logicalDevice, &layoutInfo,
                nullptr, &layout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }

            setInfo.layout = layout;
            mAllLayouts.push_back(layout); // 按set索引顺序存储
        }
    }

    // 创建描述符池（支持所有Set）
    void createDescriptorPool() {
        std::unordered_map<VkDescriptorType, uint32_t> typeCounts;

        // 统计所有Set需要的描述符类型数量
        for (const auto& [setIndex, setInfo] : mSetMap) {
            for (const auto& [binding, info] : setInfo.bindings) {
                typeCounts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] += mSwapChainCount;
            }
        }

        // 转换为VkDescriptorPoolSize
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (const auto& [type, count] : typeCounts) {
            poolSizes.push_back({ type, count });
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(mSwapChainCount * mSetMap.size());

        if (vkCreateDescriptorPool(mContext.logicalDevice, &poolInfo,
            nullptr, &mDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    // 创建所有Set的描述符集
    void createDescriptorSets() {
        // 为每个交换链图像创建所有Set
        mDescriptorSets.resize(mSwapChainCount);

        for (size_t imageIndex = 0; imageIndex < mSwapChainCount; ++imageIndex) {
            auto& imageSets = mDescriptorSets[imageIndex];

            // 为每个Set分配描述符集
            for (auto& [setIndex, setInfo] : mSetMap) {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = mDescriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &setInfo.layout;

                VkDescriptorSet descriptorSet;
                if (vkAllocateDescriptorSets(mContext.logicalDevice, &allocInfo,
                    &descriptorSet) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate descriptor sets!");
                }

                // 存储描述符集 [imageIndex][setIndex]
                imageSets[setIndex] = descriptorSet;

                // 更新描述符集写入
                std::vector<VkWriteDescriptorSet> writes;
                for (const auto& [binding, info] : setInfo.bindings) {
                    VkDescriptorBufferInfo bufferInfo =
                        info.buffers[imageIndex]->getDescriptorInfo();

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptorSet;
                    write.dstBinding = binding;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    write.descriptorCount = 1;
                    write.pBufferInfo = &bufferInfo;

                    writes.push_back(write);
                }

                vkUpdateDescriptorSets(mContext.logicalDevice,
                    static_cast<uint32_t>(writes.size()),
                    writes.data(), 0, nullptr);
            }
        }
    }

    // 更新数据时需要指定Set和Binding
    template <typename T>
    void updateUBOData(uint32_t set, uint32_t binding,
        uint32_t imageIndex, const T& data) {
        auto& setInfo = mSetMap.at(set);
        auto& bindingInfo = setInfo.bindings.at(binding);

        if (sizeof(T) != bindingInfo.uboInfo.dataSize) {
            throw std::runtime_error("UBO data size mismatch!");
        }

        auto* buffer = dynamic_cast<TypedUniformBuffer<T>*>(
            bindingInfo.buffers[imageIndex].get());

        if (!buffer) {
            throw std::runtime_error("Invalid UBO type!");
        }

        buffer->updateData(&data, sizeof(T));
    }

    bool hasSet(uint32_t set) const {
        return mSetMap.find(set) != mSetMap.end();
    }

    VkDescriptorSetLayout getSetLayout(uint32_t set) const {
        return mSetMap.at(set).layout;
    }

    VkDescriptorSet getDescriptorSet(uint32_t imageIndex, uint32_t set) const {
        return mDescriptorSets[imageIndex].at(set);
    }

    // 获取指定Set和图像的描述符集
    const std::vector<std::unordered_map<uint32_t, VkDescriptorSet>>& getDescriptorSet() const {
        return mDescriptorSets;
    }

    const std::vector<VkDescriptorSetLayout>& getAllLayouts() const {
        return mAllLayouts;
    }

    // 清理资源
    void cleanup() {
        vkDestroyDescriptorPool(mContext.logicalDevice, mDescriptorPool, nullptr);

        for (auto& [setIndex, setInfo] : mSetMap) {
            vkDestroyDescriptorSetLayout(mContext.logicalDevice,
                setInfo.layout, nullptr);
            for (auto& [binding, info] : setInfo.bindings) {
                for (auto& buffer : info.buffers) {
                    buffer->cleanup();
                }
            }
        }
        mSetMap.clear();
    }

private:
    struct UBOInfo {
        size_t dataSize;
        VkShaderStageFlags stageFlags;
    };

    struct BindingInfo {
        UBOInfo uboInfo;
        std::vector<std::unique_ptr<UniformBufferBase>> buffers;
    };

    struct SetInfo {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, BindingInfo> bindings;
    };

    VulkanContext mContext;
    size_t mSwapChainCount;
    std::map<uint32_t, SetInfo> mSetMap;
    std::vector<VkDescriptorSetLayout> mAllLayouts; 
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    std::vector<std::unordered_map<uint32_t, VkDescriptorSet>> mDescriptorSets;
};
