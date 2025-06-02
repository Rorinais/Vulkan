#include "DescriptorLayoutManager.hpp"
#include <stdexcept>

void DescriptorLayoutManager::addBinding(uint32_t set, const BindingInfo& info) {
    mSetBindings[set].push_back(info);
}

void DescriptorLayoutManager::createLayouts(const LogicalDevice::Ptr& device) {
    for (auto& [set, bindings] : mSetBindings) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        for (const auto& binding : bindings) {
            layoutBindings.push_back(binding.layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(device->getHandle(), &layoutInfo,
            nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout");
        }

        mDescriptorSetLayouts[set] = layout;
    }
}

void DescriptorLayoutManager::cleanup(const LogicalDevice::Ptr& device) {
    for (auto& [set, layout] : mDescriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(device->getHandle(), layout, nullptr);
    }
    mDescriptorSetLayouts.clear();
}