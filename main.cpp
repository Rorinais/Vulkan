#include "./renderer/core/base.hpp"
#include "./renderer/core/window/WindowManager.hpp"
#include "./renderer/core/device/InstanceManager.hpp"
#include "./renderer/core/device/DebugManager.hpp"
#include "./renderer/core/device/PhysicalDeviceSelector.hpp"
#include "./renderer/core/device/LogicalDevice.hpp"
#include "./renderer/core/rendering/ShaderManager.hpp"
#include "./renderer/core/rendering/renderPipeline/pipelines/PipelineFactory.hpp"
#include "./renderer/core/rendering/renderPipeline/passes/RenderPassBuilder.hpp"
#include "./renderer/core/swapChain/SwapChainManager.hpp"
#include "./renderer/core/rendering/CommandSystem.hpp"
#include "./renderer/core/rendering/vertexBuffer.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex> vertices = {
	{{-0.5f , -0.5f,0.0f } , {0.0f , 0.0f , 0.0f }} ,
	{{0.5f , -0.5f,0.0f } , {1.0f , 0.0f , 0.0f }} ,
	{{0.5f , 0.5f,0.0f } , {1.0f , 1.0f , 0.0f }} ,
	{{-0.5f , 0.5f,0.0f } , {0.0f , 1.0f , 0.0f }}
};
const std::vector<uint32_t> indices = {
	0, 1, 2,
	2, 3, 0
};
class HelloTriangleApplication {
public:
	HelloTriangleApplication() = default;
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}
private:
	void initWindow() {
		WindowManager::Config config{
			.width = 800,
			.height = 600,
			.title = "Vulkan Demo",
			.resizable = true,
			.monitorIndex = 0,
			.fullScreen=false,
			.highDPI=false
		};
		mWindow =new WindowManager(config);
		mWindow->setKeyCallback([this](int key, int action) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				glfwSetWindowShouldClose(mWindow->getHandle(), GLFW_TRUE);
			}
			});

		mWindow->setResizeCallback([this](int, int) {
			mFramebufferResized.store(true);
			});
	}
	void initVulkan() {
		InstanceManager::Config instanceConfig{
			.appName = "Vulkan Demo",
			.enableValidation = enableValidationLayers
		};
		mInstance = new InstanceManager(instanceConfig);

		mDebug = new DebugManager(mInstance->getHandle(), enableValidationLayers);

		mPhysicalDeviceSelector = new PhysicalDeviceSelector(mInstance->getHandle(), mWindow->createSurface(mInstance->getHandle()));

		auto indices = mPhysicalDeviceSelector->getQueueIndices(mPhysicalDeviceSelector->getPhysicalDevice());
		mLogicalDevice = new LogicalDevice(mPhysicalDeviceSelector->getPhysicalDevice(), indices, enableValidationLayers);

		mSwapChainManager = new SwapChainManager(
			mPhysicalDeviceSelector->getPhysicalDevice(),
			mLogicalDevice->getDevice(),
			mWindow->getHandle(),
			mPhysicalDeviceSelector->getSurface());

		mGraphicsPipelineFactory = new PipelineFactory(mLogicalDevice->getDevice());

		mGraphicsPipelineFactory->createRenderPass(mSwapChainManager->getSwapChainImageFormat());
		createGraphicsPipeline();

		mSwapChainManager->createFramebuffers(mGraphicsPipelineFactory->getRenderPass());

		createCommandSystem();
	}
	void mainLoop() {
		while (!glfwWindowShouldClose(mWindow->getHandle())) {
			glfwPollEvents();

			static int lastWidth = 0, lastHeight = 0;
			int newWidth, newHeight;
			glfwGetFramebufferSize(mWindow->getHandle(), &newWidth, &newHeight);

			if (newWidth != lastWidth || newHeight != lastHeight) {
				lastWidth = newWidth;
				lastHeight = newHeight;
				mFramebufferResized.store(true);
			}

			if (mFramebufferResized.load()) {
				recreateSwapChainResources();
				mFramebufferResized.store(false);
			}

			try {
				drawFrame();
			}
			catch (const std::runtime_error& e) {
				std::cerr << "Drawing failed: " << e.what() << std::endl;
				vkDeviceWaitIdle(mLogicalDevice->getDevice());
				recreateSwapChainResources();
			}
		}

		vkDeviceWaitIdle(mLogicalDevice->getDevice());
	}
	void cleanUp() {
		if (mVertexBuffer){
			mVertexBuffer->cleanup();
			delete mVertexBuffer;
			mVertexBuffer = nullptr;	
		}

		if (mCommandSystem) {
			mCommandSystem->cleanup();
			delete mCommandSystem;
			mCommandSystem = nullptr;
		}

		if (mSwapChainManager) {
			mSwapChainManager->cleanupSwapChain();
			delete mSwapChainManager;
			mSwapChainManager = nullptr;
		}

		if (mGraphicsPipelineFactory) {
			mGraphicsPipelineFactory->cleanup(); 
			delete mGraphicsPipelineFactory;
			mGraphicsPipelineFactory = nullptr;
		}

		if (mLogicalDevice) {
			delete mLogicalDevice;
			mLogicalDevice = nullptr;
		}

		if (mPhysicalDeviceSelector) {
			vkDestroySurfaceKHR(mInstance->getHandle(),
				mPhysicalDeviceSelector->getSurface(),
				nullptr);
			delete mPhysicalDeviceSelector;
			mPhysicalDeviceSelector = nullptr;
		}

		if (mDebug) {
			delete mDebug;
			mDebug = nullptr;
		}

		if (mInstance) {
			delete mInstance;
			mInstance = nullptr;
		}

		if (mWindow) {
			delete mWindow;
			mWindow = nullptr;
		}
	}
	void recreateSwapChainResources() {
		vkDeviceWaitIdle(mLogicalDevice->getDevice());

		if (mCommandSystem) {
			mCommandSystem->cleanupCommandBuffers();
		}
		if (mSwapChainManager) {
			mSwapChainManager->cleanupSwapChain();
		}

		try {
			mSwapChainManager->createSwapChain();
			mSwapChainManager->createImageViews();
			mSwapChainManager->createFramebuffers(mGraphicsPipelineFactory->getRenderPass());

			mCommandSystem->recreateCommandBuffers(
				mSwapChainManager->getSwapChainFramebuffers(),
				mGraphicsPipelineFactory->getGraphicsPipeline(),
				mSwapChainManager->getSwapChainExtent(),
				mVertexBuffer->getVertexBuffer(),
				mVertexBuffer->getVertexCount(),
				mVertexBuffer->getIndexBuffer(),
				mVertexBuffer->getIndexCount()
			);
			mCommandSystem->updateSyncObjects(mSwapChainManager->getSwapChainImageCount());
		}
		catch (const std::exception& e) {
			std::cerr << "swapChain recreation failed: " << e.what() << std::endl;
			return;
		}

		// 4. 重置帧索引
		mCurrentFrame = 0;
		mFramebufferResized.store(false);
	}
	void createGraphicsPipeline() {
		ShaderManager shaderManager(mLogicalDevice->getDevice());
		VkShaderModule vertModule = shaderManager.loadFromGLSL("resources/shaders/shader.vert", VK_SHADER_STAGE_VERTEX_BIT,"VertexShader");
		VkShaderModule fragModule = shaderManager.loadFromGLSL("resources/shaders/shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT,"FragmentShader");

		PipelineFactory::PipelineConfig config;
		config.vertexInput.addBinding(0, sizeof(Vertex))
			.addAttribute(0, { 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) })
			.addAttribute(0, { 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });

#if USE_OPENGL_COORDINATES
		config.rasterization.setCullMode(VK_CULL_MODE_BACK_BIT).setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
#endif 
		config.viewport.createViewport(mSwapChainManager->getSwapChainExtent());
		config.colorBlend.addAttachment({.blendEnable = VK_FALSE });

		mGraphicsPipelineFactory->configure(config);
		mGraphicsPipelineFactory->createGraphicsPipeline(vertModule, fragModule);
	}
	void createCommandSystem() {
		auto queueIndices = mPhysicalDeviceSelector->getQueueIndices(mPhysicalDeviceSelector->getPhysicalDevice());
		mCommandSystem = new CommandSystem(
			mLogicalDevice->getDevice(),
			mLogicalDevice->getQueues().graphicsQueue,
			mLogicalDevice->getQueues().presentQueue,
			mGraphicsPipelineFactory->getRenderPass(),
			mSwapChainManager->getSwapChainExtent(),
			MAX_FRAMES_IN_FLIGHT
		);
		mCommandSystem->createCommandPool(queueIndices);

		mVertexBuffer = new VertexBuffer<Vertex>(
			mPhysicalDeviceSelector->getPhysicalDevice(),
			mLogicalDevice->getDevice(),
			mLogicalDevice->getQueues().graphicsQueue,
			mCommandSystem->getCommandPool()
		);
		mVertexBuffer->createVertexBuffer(vertices);
		mVertexBuffer->createIndexBuffer(indices);

		mCommandSystem->createCommandBuffers(
			mSwapChainManager->getSwapChainFramebuffers(),
			mGraphicsPipelineFactory->getGraphicsPipeline(),
			mVertexBuffer->getVertexBuffer(),
			mVertexBuffer->getVertexCount(),
			mVertexBuffer->getIndexBuffer(),
			mVertexBuffer->getIndexCount()
		);
		mCommandSystem->updateSyncObjects(
			mSwapChainManager->getSwapChainImageCount()
		);
	}
	void drawFrame() {
		uint32_t imageIndex;
		bool needRecreate = false;

		bool frameSuccess = mCommandSystem->beginFrame(
			mSwapChainManager->getSwapChain(),
			imageIndex,
			needRecreate
		);

		if (!frameSuccess || needRecreate || mFramebufferResized.load()) {
			recreateSwapChainResources();
			return;
		}

		mCommandSystem->submitFrame(imageIndex);

		bool presentSuccess = mCommandSystem->presentFrame(
			mSwapChainManager->getSwapChain(),
			imageIndex,
			needRecreate
		);

		if (!presentSuccess || needRecreate || mFramebufferResized.load()) {
			recreateSwapChainResources();
		}
	}
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
private:
	WindowManager* mWindow = nullptr;
	InstanceManager* mInstance = nullptr;
	DebugManager* mDebug = nullptr;
	PhysicalDeviceSelector* mPhysicalDeviceSelector = nullptr;
	LogicalDevice* mLogicalDevice = nullptr;
	SwapChainManager* mSwapChainManager = nullptr;
	PipelineFactory* mGraphicsPipelineFactory = nullptr;
	CommandSystem* mCommandSystem = nullptr;
	VertexBuffer<Vertex>* mVertexBuffer = nullptr;

	uint32_t mCurrentFrame = 0;
	std::atomic<bool> mFramebufferResized{ false };
};

int main() {
#ifdef _WIN32
	_putenv_s("VK_LAYER_PATH", "layers");
#endif

	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

