#include "./renderer/core/base.h"
#include "./renderer/core/window/WindowManager.hpp"
#include "./renderer/core/device/InstanceManager.hpp"
#include "./renderer/core/device/DebugManager.hpp"
#include "./renderer/core/device/PhysicalDeviceSelector.hpp"
#include "./renderer/core/device/LogicalDevice.hpp"
#include "./renderer/core/rendering/ShaderManager.hpp"
#include "./renderer/core/rendering/renderPipeline/pipelines/PipelineFactory.hpp"
#include "./renderer/core/rendering/renderPipeline/passes/RenderPassBuilder.hpp"
#include "./renderer/core/swapChain/SwapChainManager.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

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
			.highDPI=true
		};
		m_window =new WindowManager(config);
		m_window->setKeyCallback([this](int key, int action) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				glfwSetWindowShouldClose(m_window->getHandle(), GLFW_TRUE);
			}
			});

		m_window->setResizeCallback([this](int, int) {
			mFramebufferResized = true;
			});
	}

	void initVulkan() {
		InstanceManager::Config instanceConfig{
			.appName = "Vulkan Demo",
			.enableValidation = enableValidationLayers
		};
		m_instance = new InstanceManager(instanceConfig);
		m_instanceValid = true;

		m_debug = new DebugManager(m_instance->getHandle(), enableValidationLayers);

		m_surface = m_window->createSurface(m_instance->getHandle());

		m_deviceSelector = new PhysicalDeviceSelector(m_instance->getHandle(), m_surface);
		m_physicalDevice = m_deviceSelector->select();
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &props);

		auto indices = m_deviceSelector->getQueueIndices(m_physicalDevice);
		m_logicalDevice = new LogicalDevice(m_physicalDevice, indices, enableValidationLayers);
		m_deviceValid = true;

		mSwapChainManager = new SwapChainManager(
			m_physicalDevice,
			m_logicalDevice->getDevice(),
			m_window->getHandle(),
			m_surface);

		createRenderPass();
		createGraphicsPipeline();
		mSwapChainManager->createFramebuffers(mRenderPass);
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		while (!m_window->shouldClose()) {
			m_window->pollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(m_logicalDevice->getDevice());
	}

	void cleanUp() {
		// 0. 清理交换链相关资源（必须在设备资源之前）
		if (mSwapChainManager) {
			delete mSwapChainManager;  // 确保内部销毁Swapchain/ImageViews/Framebuffers
			mSwapChainManager = nullptr;
		}

		// 1. 清理设备相关资源
		if (m_deviceValid) {
			// 1.1 销毁同步对象
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vkDestroySemaphore(m_logicalDevice->getDevice(), mRenderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(m_logicalDevice->getDevice(), mImageAvailableSemaphores[i], nullptr);
				vkDestroyFence(m_logicalDevice->getDevice(), mInFlightFences[i], nullptr);
			}

			// 1.2 销毁图形管线相关对象（关键新增部分）
			vkDestroyPipeline(m_logicalDevice->getDevice(), mGraphicsPipeline, nullptr);
			vkDestroyPipelineLayout(m_logicalDevice->getDevice(), mPipelineLayout, nullptr);
			vkDestroyRenderPass(m_logicalDevice->getDevice(), mRenderPass, nullptr);

			// 1.3 销毁命令池
			vkDestroyCommandPool(m_logicalDevice->getDevice(), mCommandPool, nullptr);

			// 1.5 销毁逻辑设备
			delete m_logicalDevice;
			m_logicalDevice = nullptr;
			m_deviceValid = false;
			std::cout << "Logical device destroyed\n";
		}

		// 2. 清理实例级资源
		if (m_surface) {
			vkDestroySurfaceKHR(m_instance->getHandle(), m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
			std::cout << "Surface destroyed\n";
		}

		// 3. 清理调试工具
		if (m_debug) {
			delete m_debug;
			m_debug = nullptr;
			std::cout << "Debug manager destroyed\n";
		}

		// 4. 清理Vulkan实例
		if (m_instanceValid) {
			delete m_instance;
			m_instance = nullptr;
			m_instanceValid = false;
			std::cout << "Vulkan instance destroyed\n";
		}

		// 5. 清理窗口和设备选择器
		if (m_windowValid) {
			delete m_window;
			m_window = nullptr;
			m_windowValid = false;
			std::cout << "Window closed\n";
		}

		if (m_deviceSelector) {
			delete m_deviceSelector;
			m_deviceSelector = nullptr;
			std::cout << "Device selector destroyed\n";
		}
	}

	void recreateSwapChainResources() {
		vkDeviceWaitIdle(m_logicalDevice->getDevice());

		// 1. 清理旧资源
		mSwapChainManager->cleanupSwapChain();

		// 2. 创建新交换链
		mSwapChainManager->createSwapChain();
		mSwapChainManager->createImageViews();

		// 3. 重建依赖资源
		createRenderPass();
		createGraphicsPipeline();
		mSwapChainManager->createFramebuffers(mRenderPass);

		// 4. 重建命令缓冲区
		vkFreeCommandBuffers(m_logicalDevice->getDevice(), mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data());
		createCommandBuffers();
	}

	//===================渲染管线相关=====================//
	void createRenderPass() {
		RenderPassBuilder::RenderPassConfig config{ .colorFormat = mSwapChainManager->getSwapChainImageFormat()};
		RenderPassBuilder passbuilder(m_logicalDevice->getDevice());
		mRenderPass = passbuilder.configureColorAttachment(config).build();
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_logicalDevice->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void createGraphicsPipeline() {
		ShaderManager shaderManager(m_logicalDevice->getDevice());
		VkShaderModule vertModule = shaderManager.loadFromGLSL("resources/shaders/shader.vert", VK_SHADER_STAGE_VERTEX_BIT,"VertexShader");
		VkShaderModule fragModule = shaderManager.loadFromGLSL("resources/shaders/shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT,"FragmentShader");

		//auto vertShaderCode = readFile("./resources/shaders/vert.spv");
		//auto fragShaderCode = readFile("./resources/shaders/frag.spv");

		//VkShaderModule vertModule = createShaderModule(vertShaderCode);
		//VkShaderModule fragModule = createShaderModule(fragShaderCode);

		PipelineFactory pipeline(m_logicalDevice->getDevice());
		mGraphicsPipeline = pipeline.createGraphicsPipeline(mRenderPass, mSwapChainManager->getSwapChainExtent(), vertModule, fragModule);
		mPipelineLayout = pipeline.getPipelineLayout();
	}
	//======================命令池相关======================//
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = m_deviceSelector->findQueueFamilies(m_physicalDevice,m_surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(m_logicalDevice->getDevice(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createCommandBuffers() {
		mCommandBuffers.resize(mSwapChainManager->getSwapChainFramebuffers().size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

		if (vkAllocateCommandBuffers(m_logicalDevice->getDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < mCommandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass;
			renderPassInfo.framebuffer = mSwapChainManager->getSwapChainFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapChainManager->getSwapChainExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

			vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(mCommandBuffers[i]);

			if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void drawFrame() {
		vkWaitForFences(m_logicalDevice->getDevice(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_logicalDevice->getDevice(), mSwapChainManager->getSwapChain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
			mFramebufferResized = false;
			recreateSwapChainResources(); 
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_logicalDevice->getDevice(), 1, &mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		mImagesInFlight[imageIndex] = mInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_logicalDevice->getDevice(), 1, &mInFlightFences[mCurrentFrame]);

		if (vkQueueSubmit(m_logicalDevice->getQueues().graphics, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChainManager->getSwapChain()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_logicalDevice->getQueues().present, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
			mFramebufferResized = false;
			recreateSwapChainResources();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void createSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImagesInFlight.resize(mSwapChainManager->getSwapChainImages().size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_logicalDevice->getDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_logicalDevice->getDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_logicalDevice->getDevice(), &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
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
	WindowManager* m_window = nullptr;
	InstanceManager* m_instance = nullptr;
	DebugManager* m_debug = nullptr;
	PhysicalDeviceSelector* m_deviceSelector = nullptr;
	LogicalDevice* m_logicalDevice = nullptr;

	bool m_windowValid{ false };
	bool m_instanceValid{ false };
	bool m_deviceValid{ false };

	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	// 交换链相关
	SwapChainManager* mSwapChainManager = nullptr;

	//渲染管线
	VkRenderPass mRenderPass = VK_NULL_HANDLE;
	VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
	VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

	//命令池
	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> mCommandBuffers{};

	std::vector<VkSemaphore> mImageAvailableSemaphores{};
	std::vector<VkSemaphore> mRenderFinishedSemaphores{};
	std::vector<VkFence> mInFlightFences{};
	std::vector<VkFence> mImagesInFlight{};
	size_t mCurrentFrame = 0;

	bool mFramebufferResized = false;
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


