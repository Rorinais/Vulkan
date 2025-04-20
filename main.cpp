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
#include "./renderer/core/rendering/vulkanBufferObject/VertexBufferObject.hpp"
#include "./renderer/core/rendering/vulkanBufferObject/IndexBufferObject.hpp"
#include "./renderer/core/rendering/vulkanBufferObject/UniformBufferObject.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

//const std::vector<Vertex> vertices = {
//	{{-0.5f , -0.5f,0.0f } , {0.0f , 0.0f , 0.0f }} ,
//	{{0.5f , -0.5f,0.0f } , {1.0f , 0.0f , 0.0f }} ,
//	{{0.5f , 0.5f,0.0f } , {1.0f , 1.0f , 0.0f }} ,
//	{{-0.5f , 0.5f,0.0f } , {0.0f , 1.0f , 0.0f }}
//};
//const std::vector<uint32_t> indices = {
//	0, 1, 2,
//	2, 3, 0
//};

const std::vector<Vertex> vertices = {
	// Front face  
	{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
	{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
	// Back face  
	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
	{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
	{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
};

const std::vector<uint32_t> indices = {
	// Front face  
	0, 1, 2, 2, 3, 0,
	// Back face  
	4, 5, 6, 6, 7, 4,
	// Left face  
	4, 0, 3, 3, 7, 4,
	// Right face  
	1, 5, 6, 6, 2, 1,
	// Top face  
	3, 2, 6, 6, 7, 3,
	// Bottom face  
	4, 5, 1, 1, 0, 4,
};


struct UniformBuffers {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct fraUBO {
	alignas(16) glm::vec4 cols;
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
			.fullScreen = false,
			.highDPI = false
		};
		mWindow = new WindowManager(config);
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

		auto queueIndices = mPhysicalDeviceSelector->getQueueIndices(mPhysicalDeviceSelector->getPhysicalDevice());
		mLogicalDevice = new LogicalDevice(mPhysicalDeviceSelector->getPhysicalDevice(), queueIndices, enableValidationLayers);

		mSwapChainManager = new SwapChainManager(
			mPhysicalDeviceSelector->getPhysicalDevice(),
			mLogicalDevice->getDevice(),
			mWindow->getHandle(),
			mPhysicalDeviceSelector->getSurface());

		mGraphicsPipelineFactory = new PipelineFactory(mLogicalDevice->getDevice());

		RenderPassBuilder::RenderPassConfig config{
		.colorFormat = mSwapChainManager->getSwapChainImageFormat(),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR
		};
		mGraphicsPipelineFactory->createRenderPass(config);

		mCommandSystem = new CommandSystem(
			mLogicalDevice->getDevice(),
			mLogicalDevice->getQueues().graphicsQueue,
			mLogicalDevice->getQueues().presentQueue,
			mGraphicsPipelineFactory->getRenderPass(),
			mSwapChainManager->getSwapChainExtent(),
			MAX_FRAMES_IN_FLIGHT
		);
		mCommandSystem->createCommandPool(queueIndices);

		VulkanContext vkContext{
			.physicalDevice = mPhysicalDeviceSelector->getPhysicalDevice(),
			.logicalDevice = mLogicalDevice->getDevice(),
			.commandPool = mCommandSystem->getCommandPool(),
			.graphicsQueue = mLogicalDevice->getQueues().graphicsQueue,
			.swapChainExtent = mSwapChainManager->getSwapChainExtent()
		};
		mVertexBuffer = new VertexBuffer(vkContext);
		mVertexBuffer->loadData(vertices);

		mIndexBuffer = new IndexBuffer(vkContext);
		mIndexBuffer->loadData(indices);

		mDescriptorManager = new MultiDescriptorManager(vkContext, mSwapChainManager->getSwapChainImageCount());

		// 添加多个Set和Binding
		mDescriptorManager->addUBOBinding<UniformBuffers>(0, 0, VK_SHADER_STAGE_VERTEX_BIT); // Set 0 Binding 0
		mDescriptorManager->addUBOBinding<fraUBO>(1, 0, VK_SHADER_STAGE_FRAGMENT_BIT);      // Set 1 Binding 0

		// 创建描述符资源
		mDescriptorManager->createDescriptorLayouts();
		mDescriptorManager->createDescriptorPool();
		mDescriptorManager->createDescriptorSets();

		// 获取所有Set的布局用于管线创建
		const auto& allLayouts = mDescriptorManager->getAllLayouts();

		// 创建管线布局
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(allLayouts.size());
		pipelineLayoutInfo.pSetLayouts = allLayouts.data();
		if (vkCreatePipelineLayout(mLogicalDevice->getDevice(), &pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		createGraphicsPipeline();

		mSwapChainManager->createFramebuffers(mGraphicsPipelineFactory->getRenderPass());

		mCommandSystem->createCommandBuffers(
			mSwapChainManager->getSwapChainFramebuffers(),
			mGraphicsPipelineFactory->getGraphicsPipeline(),
			mVertexBuffer->getBuffer(),
			mVertexBuffer->getVertexCount(),
			mIndexBuffer->getBuffer(),
			mIndexBuffer->getIndexCount(),
			mDescriptorManager->getDescriptorSet(),
			pipelineLayout
		);
		mCommandSystem->updateSyncObjects(
			mSwapChainManager->getSwapChainImageCount()
		);
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
		if (mDescriptorManager) {
			mDescriptorManager->cleanup();
			delete mDescriptorManager;
			mDescriptorManager = nullptr;
		}
		if (mIndexBuffer) {
			mIndexBuffer->cleanup();
			delete mIndexBuffer;
			mIndexBuffer = nullptr;
		}
		if (mVertexBuffer) {
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
				mVertexBuffer->getBuffer(),
				mVertexBuffer->getVertexCount(),
				mIndexBuffer->getBuffer(),
				mIndexBuffer->getIndexCount(),
				mDescriptorManager->getDescriptorSet()
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
		VkShaderModule vertModule = shaderManager.loadFromGLSL("resources/shaders/shader.vert", VK_SHADER_STAGE_VERTEX_BIT, "VertexShader");
		VkShaderModule fragModule = shaderManager.loadFromGLSL("resources/shaders/shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT, "FragmentShader");

		PipelineFactory::PipelineConfig config;
		config.vertexInput.addBinding(0, sizeof(Vertex))
			.addAttribute(0, { 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) })
			.addAttribute(0, { 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });

#if USE_OPENGL_COORDINATES
		config.rasterization.setCullMode(VK_CULL_MODE_BACK_BIT).setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
#endif 
		config.viewport.createViewport(mSwapChainManager->getSwapChainExtent());
		config.colorBlend.addAttachment({ .blendEnable = VK_FALSE });

		config.pipelineLayout = pipelineLayout;

		mGraphicsPipelineFactory->configure(config);
		mGraphicsPipelineFactory->createGraphicsPipeline(vertModule, fragModule);
	}
	void drawFrame() {
		uint32_t imageIndex;
		bool needRecreate = false;

		// 1. 开始帧并获取当前图像索引
		bool frameSuccess = mCommandSystem->beginFrame(
			mSwapChainManager->getSwapChain(),
			imageIndex,
			needRecreate
		);

		if (!frameSuccess || needRecreate || mFramebufferResized.load()) {
			recreateSwapChainResources();
			return;
		}

		// 2. 更新当前帧的Uniform数据 
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float>(currentTime - startTime).count();

		// 构造矩阵数据
		UniformBuffers ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));

		float aspect = mSwapChainManager->getSwapChainExtent().width /
			(float)mSwapChainManager->getSwapChainExtent().height;
		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			aspect,
			0.1f,
			10.0f);
		//ubo.proj[1][1] *= -1; 

		fraUBO fbo{};
		fbo.cols = glm::vec4(0.0f, 1.f, 0.f, 1.0f);

		try {
			mDescriptorManager->updateUBOData<UniformBuffers>(0,0, imageIndex, ubo);
			mDescriptorManager->updateUBOData<fraUBO>(1,0, imageIndex, fbo);
		}
		catch (const std::exception& e) {
			std::cerr << "Failed to update uniform buffer: " << e.what() << std::endl;
			return;
		}

		// 3. 提交绘制命令
		mCommandSystem->submitFrame(imageIndex);

		// 4. 呈现帧
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
	VertexBuffer* mVertexBuffer = nullptr;
	IndexBuffer* mIndexBuffer = nullptr;
	MultiDescriptorManager* mDescriptorManager = nullptr;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

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