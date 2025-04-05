#include "LogicalDevice.hpp"

LogicalDevice::LogicalDevice(VkPhysicalDevice physicalDevice,const QueueFamilyIndices& queueIndices,bool enableValidation){

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
	std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &mDevice);
    if (result != VK_SUCCESS) {
        std::string errorMsg;
        switch (result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY: errorMsg = "Out of host memory"; break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorMsg = "Out of device memory"; break;
        case VK_ERROR_INITIALIZATION_FAILED: errorMsg = "Initialization failed"; break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: errorMsg = "Extension not present"; break;
        case VK_ERROR_LAYER_NOT_PRESENT: errorMsg = "Layer not present"; break;
        default: errorMsg = "Unknown error"; break;
        }
        throw std::runtime_error("Failed to create logical device: " + errorMsg);
    }

    vkGetDeviceQueue(mDevice, queueIndices.graphicsFamily.value(), 0, &mQueues.graphics);
    vkGetDeviceQueue(mDevice, queueIndices.presentFamily.value(), 0, &mQueues.present);
}

LogicalDevice::~LogicalDevice() {
    if (mDevice) {
        vkDestroyDevice(mDevice, nullptr);
    }
}

