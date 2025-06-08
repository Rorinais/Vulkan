#pragma once
#include "../../base.hpp"   
#include "../platform/window.hpp" 
#include "../utils/shaderUtils.hpp"
#include "../../renderer/core/commands/commandPool.hpp"
#include "../../renderer/core/commands/commandBuffer.hpp"
#include "../../renderer/core/sync/semaphore.hpp"
#include "../../renderer/core/sync/fence.hpp"
#include "../../renderer/core/context/instance.hpp" 
#include "../../renderer/core/context/vulkanDebug.hpp" 
#include "../../renderer/core/context/physicalDevice.hpp" 
#include "../../renderer/core/context/logicalDevice.hpp" 
#include "../../renderer/core/context/swapchain.hpp"
#include "../../renderer/passes/renderPass.hpp"
#include "../../renderer/pipeline/pipelineStates/pipeline.hpp"
#include "../../renderer/pipeline/descriptors/UniformBufferManager.hpp"
#include "../../renderer/resources/textures/Texture.hpp"
#include "../../renderer/resources/buffers/Buffer.hpp"
#include "../../renderer/resources/buffers/IndexBuffer.hpp"
#include "../../renderer/resources/buffers/VertexArrayBuffer.hpp"
#include "../../renderer/resources/shaders/baseShader.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

//struct Vertex {
//    glm::vec2 pos;
//    glm::vec2 texCoord;
//};

//const std::vector<glm::vec3> positions = {
//	// Front face (0-3)
//	{-0.5f, -0.5f,  0.5f},
//	{ 0.5f, -0.5f,  0.5f},
//	{ 0.5f,  0.5f,  0.5f},
//	{-0.5f,  0.5f,  0.5f},
//
//	// Back face (4-7)
//	{ 0.5f, -0.5f, -0.5f},
//	{-0.5f, -0.5f, -0.5f},
//	{-0.5f,  0.5f, -0.5f},
//	{ 0.5f,  0.5f, -0.5f},
//
//	// Left face (8-11)
//	{-0.5f, -0.5f, -0.5f},
//	{-0.5f, -0.5f,  0.5f},
//	{-0.5f,  0.5f,  0.5f},
//	{-0.5f,  0.5f, -0.5f},
//
//	// Right face (12-15)
//	{ 0.5f, -0.5f,  0.5f},
//	{ 0.5f, -0.5f, -0.5f},
//	{ 0.5f,  0.5f, -0.5f},
//	{ 0.5f,  0.5f,  0.5f},
//
//	// Top face (16-19)
//	{-0.5f,  0.5f,  0.5f},
//	{ 0.5f,  0.5f,  0.5f},
//	{ 0.5f,  0.5f, -0.5f},
//	{-0.5f,  0.5f, -0.5f},
//
//	// Bottom face (20-23)
//	{-0.5f, -0.5f, -0.5f},
//	{ 0.5f, -0.5f, -0.5f},
//	{ 0.5f, -0.5f,  0.5f},
//	{-0.5f, -0.5f,  0.5f},
//};
//
//const std::vector<glm::vec3> colors = {
//	// Front face
//	{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f},
//	// Back face
//	{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
//	// Left face
//	{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.5f, 0.5f, 0.5f},
//	// Right face
//	{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
//	// Top face
//	{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f},
//	// Bottom face
//	{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
//};
//
//const std::vector<glm::vec2> texCoords = {
//	// Front face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//	// Back face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//	// Left face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//	// Right face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//	// Top face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//	// Bottom face
//	{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
//};
//
//const std::vector<uint32_t> indices = {
//	// Front
//	0, 1, 2, 2, 3, 0,
//	// Back
//	4, 5, 6, 6, 7, 4,
//	// Left
//	8, 9, 10, 10, 11, 8,
//	// Right
//	12, 13, 14, 14, 15, 12,
//	// Top
//	16, 17, 18, 18, 19, 16,
//	// Bottom
//	20, 21, 22, 22, 23, 20,
//};



class Application {
public:
    struct FrameData {
        uint32_t imageIndex;
        VkResult acquireResult;
    };

    Application() = default;
    ~Application() = default;

    void run();

private:
    void initWindow();
    void initVulkan();
    void createSyncObjects();
    void createRenderPass();
    void createPipeline();
    void createCommandBuffers();
	void recordCommandBuffer(size_t index);
    void recreateSwapChain();
    void cleanupSwapChain();
    void drawFrame();
    void mainLoop();
    void cleanup();

	void createGeometry();
    FrameData acquireNextImage();
    void submitCommandBuffer(uint32_t imageIndex);
    void presentFrame(uint32_t imageIndex);

private:
    Window::Ptr mWindow;
    Instance::Ptr mInstance;
    VulkanDebug::Ptr mDebug;
    PhysicalDevice::Ptr mPhysicalDevice;
    LogicalDevice::Ptr mLogicalDevice;
    SwapChain::Ptr mSwapChain;
    CommandPool::Ptr mCommandPool;
    RenderPass::Ptr mRenderPass;
    Pipeline::Ptr mPipeline;
    VertexArrayBuffer::Ptr mVertexBuffer;
    IndexBuffer::Ptr mIndexBuffer;

    std::vector<CommandBuffer::Ptr> mCommandBuffers;
    std::vector<Semaphore::Ptr> mImageAvailableSemaphores;
    std::vector<Semaphore::Ptr> mRenderFinishedSemaphores;
    std::vector<Fence::Ptr> mInFlightFences;

    BaseShader* mBaseShader;
	UniformBufferManager* mDescriptorManager = nullptr;

    int mCurrentFrame = 0;
    bool mFramebufferResized = false;
};

