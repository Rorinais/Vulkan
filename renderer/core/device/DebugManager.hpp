#pragma once
#include"../base.h"

class DebugManager {
public:
    DebugManager(VkInstance instance, bool enableValidation);
    ~DebugManager();

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;
private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    bool mEnabled;
};
