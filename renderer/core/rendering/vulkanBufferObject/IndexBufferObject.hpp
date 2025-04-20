#pragma once
#include "BufferObject.hpp"
#include <iostream>

class IndexBuffer : public BufferObject {
public:
    using BufferObject::BufferObject;

    void loadData(const std::vector<uint32_t>& indices) {
        if (indices.empty()) {
            throw std::runtime_error("Index data is empty!");
        }
        const VkDeviceSize dataSize = sizeof(uint32_t) * indices.size();
        uploadData(indices.data(), dataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        // 调试输出
        std::cout << "Index buffer handle: " << getBuffer()
            << ", size: " << getSize() << " bytes" << std::endl;
    }

    uint32_t getIndexCount() const noexcept {
        return static_cast<uint32_t>(getSize() / sizeof(uint32_t));
    }
};