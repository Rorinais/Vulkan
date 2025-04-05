#include"instanceManager.hpp"
#include "DebugManager.hpp" 

InstanceManager::InstanceManager(const Config& config):mConfig(config) {
	//if (mConfig.enableValidation) {
	//	std::cout << "Enabling validation layers:\n";
	//	for (auto& layer : validationLayers) {
	//		std::cout << "  " << layer << "\n";
	//	}
	//}
	
	if (mConfig.enableValidation && !checkValidationSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = mConfig.appVersion;
	appInfo.pApplicationName = mConfig.appName;
	appInfo.pEngineName = mConfig.engineName;
	appInfo.engineVersion = mConfig.engineVersion;
	appInfo.apiVersion = VK_API_VERSION_1_2;

	auto extensions = getRequiredExtensions();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (mConfig.enableValidation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		DebugManager::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}
	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}
InstanceManager::~InstanceManager() {
	if (mInstance != VK_NULL_HANDLE) {
		vkDestroyInstance(mInstance, nullptr); 
	}
}
//bool InstanceManager::checkValidationSupport() const {
//	uint32_t layerCount;
//	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//
//	std::vector<VkLayerProperties>availableLayers(layerCount);
//	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//
//	for (const char* layerName : validationLayers) {
//		bool layerFound = false;
//
//		for (const auto& layerProperties : availableLayers) {
//			if (strcmp(layerName, layerProperties.layerName) == 0) {
//				layerFound = true;
//				break;
//			}
//		}
//		if (!layerFound) {
//			return false;
//		}
//	}
//	return true;
//}
// InstanceManager.cpp
bool InstanceManager::checkValidationSupport() const{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// ��ӡ���п��ò�
	std::cerr << "===== Available Vulkan Layers =====" << std::endl;
	for (const auto& layer : availableLayers) {
		std::cerr << "Layer: " << layer.layerName
			<< "\n\tSpec Version: " << layer.specVersion
			<< "\n\tImpl Version: " << layer.implementationVersion
			<< "\n\tDescription: " << layer.description << std::endl;
	}

	// �������Ĳ��Ƿ����
	bool allLayersFound = true;
	for (const char* layerName : validationLayers) {
		bool found = false;
		for (const auto& layer : availableLayers) {
			if (strcmp(layerName, layer.layerName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			std::cerr << "[ERROR] Layer not found: " << layerName << std::endl;
			allLayersFound = false;
		}
	}
	return allLayersFound;
}

//std::vector<const char*> InstanceManager::getRequiredExtensions() const {
//	uint32_t glfwExtensionCount = 0;
//	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//
//	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//	if (mConfig.enableValidation)
//	{
//		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//	}
//	return extensions;
//}
std::vector<const char*> InstanceManager::getRequiredExtensions() const {
	std::vector<const char*> extensions;

	// GLFW��չ
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

	// ǿ�����õ�����չ
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}