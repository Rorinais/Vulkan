#pragma once
#include"../../base.hpp"
#include "../../core/platform/window.hpp" 
#include"logicalDevice.hpp"

class SwapChain {
public:
	using Ptr = std::shared_ptr<SwapChain>;
	static Ptr create(const LogicalDevice::Ptr& logicalDevice, const Window::Ptr& window){
		return std::make_shared<SwapChain>(logicalDevice,window); 
	}

	SwapChain(const LogicalDevice::Ptr& logicalDevice,const Window::Ptr& window);
	~SwapChain();

	void createSwapChain();
	void createImageViews();
	void createFramebuffers(VkRenderPass renderPass);

	//void createDepthResources();

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSwapchainKHR& getHandle() { return mSwapChain; }
	std::vector<VkImage>& getSwapChainImages() { return mSwapChainImages; }
	std::vector<VkImageView>& getSwapChainImageViews() { return mSwapChainImageViews; }
	std::vector<VkFramebuffer>& getSwapChainFramebuffers() { return mSwapChainFramebuffers; }
	VkFormat& getSwapChainImageFormat() { return mSwapChainImageFormat; }
	const VkExtent2D& getSwapChainExtent() const { return mSwapChainExtent; }
private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
private:
	LogicalDevice::Ptr mLogicalDevice;
	Window::Ptr mWindow;

	VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> mSwapChainImages{};
	std::vector<VkImageView> mSwapChainImageViews{};
	std::vector<VkFramebuffer> mSwapChainFramebuffers{};
	VkFormat mSwapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D mSwapChainExtent = { 0, 0 };

	//std::unique_ptr<Texture> mDepthTexture;
};