#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <functional>
#include <set>
#include <vector>
#include <string>
#include <chrono>

#include"macros.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif 

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> presentModes{};
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

