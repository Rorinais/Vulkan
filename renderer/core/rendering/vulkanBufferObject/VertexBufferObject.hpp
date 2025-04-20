#pragma once	
#include "BufferObject.hpp"
class VertexBuffer : public BufferObject {
public:
    using BufferObject::BufferObject;

    template <typename VertexType>
    void loadData(const std::vector<VertexType>& vertices) {
        const VkDeviceSize dataSize = sizeof(VertexType) * vertices.size();
        uploadData(vertices.data(), dataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    const VulkanContext& getContext() const {
        return mContext;
    }

	uint32_t getVertexCount() const noexcept {
		return static_cast<uint32_t>(getSize() / sizeof(Vertex));
	}
};