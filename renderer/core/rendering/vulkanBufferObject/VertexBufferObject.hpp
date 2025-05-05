#include "BufferObject.hpp"
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream> // 用于调试输出

class VertexBuffer {
public:
    struct AttributeData {
        std::vector<uint8_t> data;  // 原始字节数据
        VkFormat format;            // 数据格式
        uint32_t location;          // 着色器中的location
        uint32_t elementSize;       // 每个元素字节数
    };

    struct BindingInfo {
        uint32_t binding;           // 绑定编号
        uint32_t stride;            // 顶点间距（自动或手动设置）
        std::vector<AttributeData> attributes; // 属性列表
        BufferObject buffer;        // 对应的缓冲区对象
    };

    explicit VertexBuffer(const VulkanContext& context)
        : mContext(context) {
    }

    ~VertexBuffer() { cleanup(); }

    void cleanup() {
        mBindings.clear();
        mBindingDescriptions.clear();
        mAttributeDescriptions.clear();
    }

    // 开始定义顶点绑定
    void beginBinding(uint32_t binding, uint32_t stride = 0) {
        mCurrentBinding = binding;
        mBindings[mCurrentBinding].buffer = BufferObject(mContext);
        mBindings[binding].binding = binding;
        mBindings[binding].stride = stride; // 0表示自动计算
        mBindings[binding].attributes.clear();

    }

    // 添加顶点属性数据
    template<typename T>
    void addAttribute(uint32_t location, VkFormat format, const std::vector<T>& data) {
        auto& binding = mBindings[mCurrentBinding];

        // 计算属性元素大小
        const uint32_t elementSize = getFormatSize(format);
        if (elementSize != sizeof(T)) {
            throw std::runtime_error("Data type size does not match format size");
        }

        // 保存属性数据
        AttributeData attr;
        attr.format = format;
        attr.location = location;
        attr.elementSize = elementSize;
        attr.data.resize(data.size() * elementSize);
        memcpy(attr.data.data(), data.data(), attr.data.size());

        binding.attributes.push_back(std::move(attr));
    }

    // 完成当前绑定，生成缓冲区
    void finishBinding() {
        auto& binding = mBindings[mCurrentBinding];
        if (binding.attributes.empty()) return;

        // 验证属性一致性
        validateAttributes(binding);

        // 计算布局参数
        calculateLayout(binding);

        // 生成交错数据
        std::vector<uint8_t> bufferData = interleaveData(binding);

        // 创建GPU缓冲区
        createGPUBuffer(binding, bufferData);

        // 生成Vulkan描述信息
        generateVulkanDescriptions(binding);
    }

    // 获取缓冲区句柄列表（按绑定顺序）
    std::vector<VkBuffer> getBufferHandles() const {
        std::vector<VkBuffer> handles;
        for (const auto& [id, binding] : mBindings) {
            handles.push_back(binding.buffer.getBuffer());
        }
        return handles;
    }

    // 获取顶点输入绑定描述
    const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() const {
        return mBindingDescriptions;
    }

    // 获取顶点属性描述
    const std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions() const {
        return mAttributeDescriptions;
    }

    // 获取顶点总数
    uint32_t getVertexCount() const {
        if (mBindings.empty()) return 0;
        const auto& firstBinding = mBindings.begin()->second;
        if (firstBinding.attributes.empty()) return 0;
        return static_cast<uint32_t>(
            firstBinding.attributes[0].data.size() /
            firstBinding.attributes[0].elementSize
            );
    }

private:
    // 验证属性数据一致性
    void validateAttributes(const BindingInfo& binding) {
        const size_t vertexCount = binding.attributes[0].data.size() /
            binding.attributes[0].elementSize;

        for (const auto& attr : binding.attributes) {
            const size_t currentCount = attr.data.size() / attr.elementSize;
            if (currentCount != vertexCount) {
                throw std::runtime_error(
                    "Attribute data count mismatch in binding " +
                    std::to_string(binding.binding) +
                    ": expected " + std::to_string(vertexCount) +
                    ", got " + std::to_string(currentCount)
                );
            }
        }
    }

    // 计算布局参数（偏移、stride等）
    void calculateLayout(BindingInfo& binding) {
        // 自动计算stride
        if (binding.stride == 0) {
            binding.stride = 0;
            for (const auto& attr : binding.attributes) {
                binding.stride += attr.elementSize;
            }
        }

        // 验证手动设置的stride
        uint32_t minStride = 0;
        for (const auto& attr : binding.attributes) {
            minStride += attr.elementSize;
        }
        if (binding.stride < minStride) {
            throw std::runtime_error(
                "Stride " + std::to_string(binding.stride) +
                " is too small for attributes in binding " +
                std::to_string(binding.binding) +
                ", minimum is " + std::to_string(minStride)
            );
        }
    }

    // 生成交错数据
    std::vector<uint8_t> interleaveData(const BindingInfo& binding) {
        const size_t vertexCount = binding.attributes[0].data.size() /
            binding.attributes[0].elementSize;
        std::vector<uint8_t> bufferData(vertexCount * binding.stride);

        // 为每个顶点设置数据
        for (size_t i = 0; i < vertexCount; ++i) {
            uint32_t offset = 0;
            for (const auto& attr : binding.attributes) {
                const uint8_t* src = attr.data.data() + i * attr.elementSize;
                uint8_t* dst = bufferData.data() + i * binding.stride + offset;
                memcpy(dst, src, attr.elementSize);
                offset += attr.elementSize;
            }
        }

        return bufferData;
    }

    // 创建GPU缓冲区
    void createGPUBuffer(BindingInfo& binding, const std::vector<uint8_t>& data) {
        try {
            binding.buffer.uploadData(
                data.data(),
                data.size(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            );
        }
        catch (const std::exception& e) {
            throw std::runtime_error(
                "Failed to create vertex buffer for binding " +
                std::to_string(binding.binding) + ": " + e.what()
            );
        }
    }

    // 生成Vulkan描述符
    void generateVulkanDescriptions(const BindingInfo& binding) {
        // 绑定描述
        mBindingDescriptions.push_back({
            .binding = binding.binding,
            .stride = binding.stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            });

        // 属性描述
        uint32_t offset = 0;
        for (const auto& attr : binding.attributes) {
            mAttributeDescriptions.push_back({
                .location = attr.location,
                .binding = binding.binding,
                .format = attr.format,
                .offset = offset
                });
            offset += attr.elementSize;
        }
    }

    // 获取格式字节大小
    static uint32_t getFormatSize(VkFormat format) {
        switch (format) {
        case VK_FORMAT_R32_SFLOAT: return 4;
        case VK_FORMAT_R32G32_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32_SFLOAT: return 12;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
        default:
            throw std::runtime_error("Unsupported vertex format: " +
                std::to_string(format));
        }
    }

    VulkanContext mContext;
    uint32_t mCurrentBinding = 0;
    std::map<uint32_t, BindingInfo> mBindings;
    std::vector<VkVertexInputBindingDescription> mBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;
};