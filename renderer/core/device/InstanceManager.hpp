#pragma once
#include"../base.h"

class InstanceManager {
public:
    struct Config {
        const char* appName = "Vulkan App";
        uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0);
        const char* engineName = "No Engine";
        uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
        bool enableValidation = true;
    };

    InstanceManager(const Config& config);
    ~InstanceManager();

    VkInstance getHandle() const { return mInstance; }

private:
    bool checkValidationSupport() const;
    std::vector<const char*> getRequiredExtensions() const;

    Config mConfig;
    VkInstance mInstance = VK_NULL_HANDLE;
};