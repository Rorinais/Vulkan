#pragma once
#include "BindingInfo.hpp"
#include "../../../base.hpp"
#include"../../core/context/logicalDevice.hpp"
#include <map>
#include <vector>

class DescriptorLayoutManager {
public:
    void addBinding(uint32_t set, const BindingInfo& info);
    void createLayouts(const LogicalDevice::Ptr& device);
    void cleanup(const LogicalDevice::Ptr& device);

    const std::map<uint32_t, VkDescriptorSetLayout>& getLayouts() const { return mDescriptorSetLayouts; }
    const std::map<uint32_t, std::vector<BindingInfo>>& getBindings() const { return mSetBindings; }

private:
    std::map<uint32_t, std::vector<BindingInfo>> mSetBindings;
    std::map<uint32_t, VkDescriptorSetLayout> mDescriptorSetLayouts;
};