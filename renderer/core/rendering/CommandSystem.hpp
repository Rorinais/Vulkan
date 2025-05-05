#pragma once
#include "../base.hpp"

class CommandSystem {
public:
    using DescriptorSetCollection = std::vector<std::vector<VkDescriptorSet>>;

    struct FrameContext {
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence inFlightFence = VK_NULL_HANDLE;;
        VkFence imageFence = VK_NULL_HANDLE; 
    };

    CommandSystem(
        VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkRenderPass renderPass,
        VkExtent2D swapChainExtent,
        uint32_t maxFramesInFlight
    );
    ~CommandSystem();

    void createCommandPool(QueueFamilyIndices queueFamilyIndices);
    void createCommandBuffers(
        const std::vector<VkFramebuffer>& framebuffers,
        VkPipeline graphicsPipeline,
        const std::vector<VkBuffer>& vertexBuffers,
        const uint32_t vertexCount,
        VkBuffer indexBuffer,
        const uint32_t indexCount,
        const std::vector<std::vector<VkDescriptorSet>>& descriptorSetsPerFrame,
        VkPipelineLayout pipelineLayout
    );




    void recreateCommandBuffers(
        const std::vector<VkFramebuffer>& framebuffers,
        VkPipeline graphicsPipeline,
        VkExtent2D newExtent,
        const std::vector<VkBuffer>& vertexBuffers,
        const uint32_t vertexCount,
        VkBuffer& indexBuffer,
        const uint32_t indexCount,
        const std::vector<std::vector<VkDescriptorSet>>& descriptorSetsPerFrame);

    void createSyncObjects(uint32_t swapChainImageCount);

    bool beginFrame(VkSwapchainKHR swapChain, uint32_t& imageIndex, bool& needRecreate);
    void submitFrame(uint32_t imageIndex);
    bool presentFrame(VkSwapchainKHR swapChain, uint32_t imageIndex, bool& needRecreate);

    void cleanup();
    void destroySyncObjects();

    void cleanupCommandBuffers();

    void updateSyncObjects(size_t imageCount);

    VkCommandPool getCommandPool() const { return mCommandPool; }

private:
    void recordCommandBuffer(
        VkCommandBuffer commandBuffer,
        VkFramebuffer framebuffer,
        VkPipeline graphicsPipeline,
        const std::vector<VkBuffer>& vertexBuffers,
        const uint32_t vertexCount,
        VkBuffer indexBuffer,
        const uint32_t indexCount,
        const std::vector<VkDescriptorSet>& descriptorSet
    ) const;

    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VkExtent2D mSwapChainExtent{};
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCommandBuffers{};

    std::vector<FrameContext> mFrameContexts{};
    std::vector<VkFence> mImagesInFlight{};
    size_t mCurrentFrame = 0;
    const uint32_t mMaxFramesInFlight=2;
};

