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

/*const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f,  0.5f},	{ 1.0f, 0.0f, 0.0f }},
	{{ 0.5f, -0.5f,  0.5f},	{ 0.0f, 1.0f, 0.0f }},
	{{ 0.5f,  0.5f,  0.5f},	{ 0.0f, 0.0f, 1.0f }},
	{{-0.5f,  0.5f,  0.5f},	{ 1.0f, 1.0f, 0.0f }},
											 	
	{{-0.5f, -0.5f, -0.5f},	{ 1.0f, 0.0f, 1.0f }},
	{{ 0.5f, -0.5f, -0.5f},	{ 0.0f, 1.0f, 1.0f }},
	{{ 0.5f,  0.5f, -0.5f},	{ 1.0f, 1.0f, 1.0f }},
	{{-0.5f,  0.5f, -0.5f},	{ 0.5f, 0.5f, 0.5f }},
};	*/											 
const std::vector<glm::vec3> positions = {
	{-0.5f, -0.5f,  0.5f}, 
	{ 0.5f, -0.5f,  0.5f}, 
	{ 0.5f,  0.5f,  0.5f}, 
	{-0.5f,  0.5f,  0.5f}, 

	{-0.5f, -0.5f, -0.5f}, 
	{ 0.5f, -0.5f, -0.5f}, 
	{ 0.5f,  0.5f, -0.5f}, 
	{-0.5f,  0.5f, -0.5f}, 
};
const std::vector<glm::vec3> colors = {
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f },

	{ 1.0f, 0.0f, 1.0f },
	{ 0.0f, 1.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ 0.5f, 0.5f, 0.5f },
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0,	// Front face  
	4, 5, 6, 6, 7, 4,	// Back face  
	4, 0, 3, 3, 7, 4,	// Left face  
	1, 5, 6, 6, 2, 1,	// Right face  
	3, 2, 6, 6, 7, 3,	// Top face  
	4, 5, 1, 1, 0, 4,	// Bottom face  
};

struct UniformBuffers {
	alignas(16) glm::mat4 model;

	alignas(16) glm::mat4 view;

	alignas(16) glm::mat4 proj;
};

class Application {
public:
	Application() = default;
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}
private:
	void initWindow();
	void mainLoop();
	void cleanUp();
	void createGraphicsPipeline();
	void recreateSwapChainResources();

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

		vkContext={
			.physicalDevice = mPhysicalDeviceSelector->getPhysicalDevice(),
			.logicalDevice = mLogicalDevice->getDevice(),
			.commandPool = mCommandSystem->getCommandPool(),
			.graphicsQueue = mLogicalDevice->getQueues().graphicsQueue,
			.swapChainExtent = mSwapChainManager->getSwapChainExtent()
		};

		//mVertexBuffer = new VertexBuffer(vkContext);
		//std::vector<glm::vec3> position;
		//std::vector<glm::vec3> color;
		//position.reserve(vertices.size());
		//color.reserve(vertices.size());
		//for (const auto& vertex : vertices) {
		//	position.push_back(vertex.position);
		//	color.push_back(vertex.color);
		//}

		//mVertexBuffer->beginBinding(0); 
		//mVertexBuffer->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, position);
		//mVertexBuffer->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, color);
		//mVertexBuffer->finishBinding();

		mVertexBuffer = new VertexBuffer(vkContext);
		mVertexBuffer->beginBinding(0); 
		mVertexBuffer->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, positions);
		mVertexBuffer->finishBinding();

		mIndexBuffer = new IndexBuffer(vkContext);
		mIndexBuffer->loadData(indices);

		mDescriptorManager = new UniformBufferManager(vkContext);
		mDescriptorManager->addUniformBinding<UniformBuffers>(0, 0, VK_SHADER_STAGE_VERTEX_BIT);


		mDescriptorManager->createDescriptorResources(mSwapChainManager->getSwapChainImageCount());

		const auto descriptorLayouts = mDescriptorManager->getDescriptorSetLayouts();
		const auto setNumbers = mDescriptorManager->getDescriptorSetNumbers();

		// 确保布局顺序正确
		std::vector<VkDescriptorSetLayout> orderedLayouts;
		for (uint32_t set : setNumbers) {
			orderedLayouts.push_back(mDescriptorManager->getDescriptorSetLayouts()[set]);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(orderedLayouts.size());
		pipelineLayoutInfo.pSetLayouts = orderedLayouts.data();
		if (vkCreatePipelineLayout(mLogicalDevice->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		createGraphicsPipeline();

		mSwapChainManager->createFramebuffers(mGraphicsPipelineFactory->getRenderPass());

		std::vector<std::vector<VkDescriptorSet>> descriptorSetss;
		const auto& allSets = mDescriptorManager->getDescriptorSetss();
		for (size_t i = 0; i < mSwapChainManager->getSwapChainImageCount(); ++i) {
			std::vector<VkDescriptorSet> frameSets;
			for (const auto& set : allSets) {
				frameSets.push_back(set[i]);
			}
			descriptorSetss.push_back(frameSets);
		}

		mCommandSystem->createCommandBuffers(
			mSwapChainManager->getSwapChainFramebuffers(),
			mGraphicsPipelineFactory->getGraphicsPipeline(),
			mVertexBuffer->getBufferHandles(),
			mVertexBuffer->getVertexCount(),
			mIndexBuffer->getBuffer(),
			mIndexBuffer->getIndexCount(),
			descriptorSetss,
			pipelineLayout
		);
		mCommandSystem->updateSyncObjects(
			mSwapChainManager->getSwapChainImageCount()
		);
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
			glm::vec3(0.0f, 0.0f, 1.5f));

		float aspect = mSwapChainManager->getSwapChainExtent().width /
			(float)mSwapChainManager->getSwapChainExtent().height;
		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			aspect,
			0.1f,
			10.0f);
		//ubo.proj[1][1] *= -1; 

		mDescriptorManager->updateUniformData<UniformBuffers>(0, 0, imageIndex, ubo);


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
	UniformBufferManager* mDescriptorManager = nullptr;

	VulkanContext vkContext;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	uint32_t mCurrentFrame = 0;
	std::atomic<bool> mFramebufferResized{ false };

};
int main() {
#ifdef _WIN32
	_putenv_s("VK_LAYER_PATH", "layers");
#endif

	Application app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void Application::initWindow() {
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

void Application::mainLoop() {
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

void Application::cleanUp() {
	vkDeviceWaitIdle(mLogicalDevice->getDevice());

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

	if (mDescriptorManager) {
		mDescriptorManager->cleanupResources();
		delete mDescriptorManager;
		mDescriptorManager = nullptr;
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

void Application::createGraphicsPipeline() {
	ShaderManager shaderManager(mLogicalDevice->getDevice());
	VkShaderModule vertModule = shaderManager.loadFromGLSL("resources/shaders/shader.vert", VK_SHADER_STAGE_VERTEX_BIT, "VertexShader");
	VkShaderModule fragModule = shaderManager.loadFromGLSL("resources/shaders/shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT, "FragmentShader");

	PipelineFactory::PipelineConfig config;

	auto bindings = mVertexBuffer->getBindingDescriptions();
	auto attributes = mVertexBuffer->getAttributeDescriptions();

	config.vertexInput = VertexInputConfig();
	for (const auto& binding : bindings) {
		config.vertexInput.addBinding(binding.binding, binding.stride);
	}
	for (const auto& attr : attributes) {
		config.vertexInput.addAttribute(attr.binding, attr);
	}

#if USE_OPENGL_COORDINATES
	config.rasterization.setCullMode(VK_CULL_MODE_BACK_BIT).setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
#endif 
	config.viewport.createViewport(mSwapChainManager->getSwapChainExtent());
	config.colorBlend.addAttachment({ .blendEnable = VK_FALSE });

	config.pipelineLayout = pipelineLayout;

	mGraphicsPipelineFactory->configure(config);
	mGraphicsPipelineFactory->createGraphicsPipeline(vertModule, fragModule);
}

void Application::recreateSwapChainResources() {
	vkDeviceWaitIdle(mLogicalDevice->getDevice());

	if (mCommandSystem) {
		mCommandSystem->cleanupCommandBuffers();
	}
	if (mSwapChainManager) {
		mSwapChainManager->cleanupSwapChain();
	}

	mDescriptorManager->recreateResources(mSwapChainManager->getSwapChainImageCount());

	try {
		mSwapChainManager->createSwapChain();
		mSwapChainManager->createImageViews();
		mSwapChainManager->createFramebuffers(mGraphicsPipelineFactory->getRenderPass());

		std::vector<std::vector<VkDescriptorSet>> descriptorSetss;
		const auto& allSets = mDescriptorManager->getDescriptorSetss();
		for (size_t i = 0; i < mSwapChainManager->getSwapChainImageCount(); ++i) {
			std::vector<VkDescriptorSet> frameSets;
			for (const auto& set : allSets) {
				frameSets.push_back(set[i]);
			}
			descriptorSetss.push_back(frameSets);
		}

		mCommandSystem->recreateCommandBuffers(
			mSwapChainManager->getSwapChainFramebuffers(),
			mGraphicsPipelineFactory->getGraphicsPipeline(),
			mSwapChainManager->getSwapChainExtent(),
			mVertexBuffer->getBufferHandles(),
			mVertexBuffer->getVertexCount(),
			mIndexBuffer->getBuffer(),
			mIndexBuffer->getIndexCount(),
			descriptorSetss
		);
		mCommandSystem->updateSyncObjects(mSwapChainManager->getSwapChainImageCount());
	}
	catch (const std::exception& e) {
		std::cerr << "swapChain recreation failed: " << e.what() << std::endl;
		return;
	}

	mCurrentFrame = 0;
	mFramebufferResized.store(false);
}