#pragma once
#include <vulkan/vulkan.h>
#include <string>

struct BindingInfo {
    VkDescriptorSetLayoutBinding layoutBinding{};
    size_t dataSize = 0;
    std::string texturePath;
};