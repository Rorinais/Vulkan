#include"PhysicalDeviceSelector.hpp"
#include"../swapChain/SwapChainManager.hpp"

PhysicalDeviceSelector::PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface)
    : mInstance(instance), m_surface(surface) {
}

VkPhysicalDevice PhysicalDeviceSelector::select() const {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isSuitable(device)) {
            return device; 
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU!");
}

QueueFamilyIndices PhysicalDeviceSelector::getQueueIndices(VkPhysicalDevice device) const {
    return findQueueFamilies(device,m_surface);
}

bool PhysicalDeviceSelector::isSuitable(VkPhysicalDevice device) const {
    QueueFamilyIndices indices = findQueueFamilies(device,m_surface);

    //std::cout << "Checking device suitability:\n"
    //    << "  Graphics queue: " << indices.graphicsFamily.has_value() << "\n"
    //    << "  Present queue: " << indices.presentFamily.has_value() << "\n";

    if (!indices.isComplete()) {
        std::cout << "  Missing required queue families\n";
        return false;
    }
    bool extensionsSupported = checkExtensions(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = SwapChainManager::querySwapChainSupport(device,m_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
            !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool PhysicalDeviceSelector::checkExtensions(VkPhysicalDevice device) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices PhysicalDeviceSelector::findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface){
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.graphicsFamily == indices.presentFamily) {
            break;
        }
    }

    return indices;
}

