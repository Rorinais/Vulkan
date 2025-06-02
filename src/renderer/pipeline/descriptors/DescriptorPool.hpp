#pragma once
#include "../../../base.hpp"
#include"../../core/context/logicalDevice.hpp"

class DescriptorPool {
public:
	using Ptr = std::shared_ptr<DescriptorPool>;
	static Ptr create(
		const LogicalDevice::Ptr& logicalDevice,
		const std::vector<VkDescriptorPoolSize>& poolSizes,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags flags = 0
	) {
		return std::make_shared<DescriptorPool>(logicalDevice, poolSizes, maxSets, flags);
	}

	DescriptorPool(
		const LogicalDevice::Ptr& logicalDevice,
		const std::vector<VkDescriptorPoolSize>& poolSizes,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags flags = 0);
	~DescriptorPool();

	VkDescriptorPool getHandle() { return mDescriptorPool; }
	LogicalDevice::Ptr getLogicalDevice() { return mLogicalDevice; }

private:
	LogicalDevice::Ptr mLogicalDevice;
	VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
};
