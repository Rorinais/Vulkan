#pragma once
#include <vulkan/vulkan.h>
#include <vector>

template <typename T>
class VertexBuffer {
public:
	VertexBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VkQueue graphicsQueue,
		VkCommandPool commandPool
	) : mPhysicalDevice(physicalDevice),
		mLogicalDevice(logicalDevice),
		mGraphicsQueue(graphicsQueue),
		mCommandPool(commandPool) {
	}

	~VertexBuffer() {
		cleanup();
	}

	void createVertexBuffer(const std::vector<T>& vertices) {
		mVertexCount = vertices.size();
		VkDeviceSize bufferSize = sizeof(T) * mVertexCount;

		// 创建暂存缓冲区
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		// 映射并拷贝数据
		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		// 创建设备本地缓冲区
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mVertexBuffer,
			mVertexBufferMemory
		);

		// 执行拷贝操作
		copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

		// 清理暂存资源
		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer(const std::vector<uint32_t>& indices) {
		mIndexCount = indices.size();
		VkDeviceSize bufferSize = sizeof(indices[0]) * mIndexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(mLogicalDevice, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			mIndexBuffer, mIndexBufferMemory);

		copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

		vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(mLogicalDevice, stagingBufferMemory, nullptr);
	}

	VkBuffer& getVertexBuffer() { return mVertexBuffer; }
	VkDeviceMemory& getVertexMemory() { return mVertexBufferMemory; }
	uint32_t getVertexCount() const { return mVertexCount; }

	VkBuffer& getIndexBuffer() { return mIndexBuffer; }
	VkDeviceMemory& getIndexMemory() { return mIndexBufferMemory; }

	uint32_t getIndexCount() const { return mIndexCount; }

	void cleanup() {
		if (mIndexBuffer != VK_NULL_HANDLE){
			vkDestroyBuffer(mLogicalDevice, mIndexBuffer, nullptr);
			mIndexBuffer = VK_NULL_HANDLE;
		}
		if (mIndexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mLogicalDevice, mIndexBufferMemory, nullptr);
			mIndexBufferMemory = VK_NULL_HANDLE;
		}
		if (mVertexBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(mLogicalDevice, mVertexBuffer, nullptr);
			mVertexBuffer = VK_NULL_HANDLE;
		}
		if (mVertexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mLogicalDevice, mVertexBufferMemory, nullptr);
			mVertexBufferMemory = VK_NULL_HANDLE;
		}
	}

private:
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory
	) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(mLogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mLogicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		vkBindBufferMemory(mLogicalDevice, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandBuffer);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("Failed to find suitable memory type!");
	}

private:
	VkPhysicalDevice mPhysicalDevice;
	VkDevice mLogicalDevice;
	VkQueue mGraphicsQueue;
	VkCommandPool mCommandPool;

	uint32_t mVertexCount = 0;
	VkBuffer mVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mVertexBufferMemory = VK_NULL_HANDLE;

	uint32_t mIndexCount = 0;
	VkBuffer mIndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory mIndexBufferMemory = VK_NULL_HANDLE;
};


