#include "InputAssembly.hpp"

InputAssembly& InputAssembly::setTopology(VkPrimitiveTopology topology) {
    mConfig.topology = topology;
    return *this;
}

InputAssembly& InputAssembly::enablePrimitiveRestart(VkBool32 enable) {
    mConfig.primitiveRestartEnable = enable;
    return *this;
}

const VkPipelineInputAssemblyStateCreateInfo& InputAssembly::getCreateInfo() const {
    static VkPipelineInputAssemblyStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = mConfig.topology;
    info.primitiveRestartEnable = mConfig.primitiveRestartEnable;
    return info;
}