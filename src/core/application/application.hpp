//#pragma once
//#include "../../base.hpp"   
//#include "../../renderer/core/FrameContext/FrameContext.hpp"
//#include "../../renderer/core/VulkanCore/VulkanCore.hpp"
//#include "../../renderer/core/WindowContext/WindowContext.hpp"
//#include "../../renderer/pipeline/descriptors/UniformBufferManager.hpp"
//#include "../../renderer/pipeline/pipelineStates/pipeline.hpp"
//#include "../../renderer/resources/Resource.hpp"
//#include "../../renderer/passes/renderPass.hpp"
//#include "../../renderer/core/app.hpp"
//
//const int MAX_FRAMES_IN_FLIGHT = 2;
//
//class Application {
//public:
//    struct FrameData {
//        uint32_t imageIndex;
//        VkResult acquireResult;
//    };
//
//    Application() {};
//    ~Application() = default;
//
//    void run();
//
//private:
//    void initWindow();
//    void initVulkan();
//    void createSyncObjects();
//    void createRenderPass();
//    void createPipeline();
//    void createCommandBuffers();
//    void recreateSwapChain();
//    void cleanupSwapChain();
//    void drawFrame();
//    void mainLoop();
//    void cleanup();
//
//	//void createGeometry();
//    FrameData acquireNextImage();
//    void submitCommandBuffer(uint32_t imageIndex);
//    void presentFrame(uint32_t imageIndex);
//
//private:
//    Window::Ptr mWindow;
//    Instance::Ptr mInstance;
//    VulkanDebug::Ptr mDebug;
//    PhysicalDevice::Ptr mPhysicalDevice;
//    LogicalDevice::Ptr mLogicalDevice;
//    SwapChain::Ptr mSwapChain;
//    CommandPool::Ptr mCommandPool;
//    RenderPass::Ptr mRenderPass;
//    Pipeline::Ptr mPipeline;
//    VertexArrayBuffer::Ptr mVertexBuffer;
//    IndexBuffer::Ptr mIndexBuffer;
//
//	Mesh mMesh;
//	MaterialWarehouse mMaterialWarehouse;
//
//
//    std::vector<CommandBuffer::Ptr> mCommandBuffers;
//    std::vector<Fence::Ptr> mInFlightFences;
//    std::vector<Semaphore::Ptr> mImageAvailableSemaphores;
//    std::vector<Semaphore::Ptr> mRenderFinishedSemaphores;
//
//
//	UniformBufferManager* mDescriptorManager = nullptr;
//
//    int mCurrentFrame = 0;
//    bool mFramebufferResized = false;
//};
//
