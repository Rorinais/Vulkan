#pragma once
#include "Buffer.hpp"
#include <iostream>

class IndexBuffer : public Buffer {
public:
    using Buffer::Buffer;

    void loadData(const std::vector<uint32_t>& indices) {
        if (indices.empty()) {
            throw std::runtime_error("Index data is empty!");
        }
        const VkDeviceSize dataSize = sizeof(uint32_t) * indices.size();
        uploadData(indices.data(), dataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    uint32_t getIndexCount() const noexcept {
        return static_cast<uint32_t>(getSize() / sizeof(uint32_t));
    }
};