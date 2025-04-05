#pragma once
#include"../base.h"

class SwapChainManager {
public:
	SwapChainManager(
		VkPhysicalDevice physicalDevice, 
		VkDevice device, 
		GLFWwindow* window, 
		VkSurfaceKHR surface);
	~SwapChainManager();

	VkSwapchainKHR getSwapChain() { return mSwapChain; }
	std::vector<VkImage> getSwapChainImages() { return mSwapChainImages; }
	std::vector<VkImageView> getSwapChainImageViews() { return mSwapChainImageViews; }
	std::vector<VkFramebuffer> getSwapChainFramebuffers() { return mSwapChainFramebuffers; }
	VkFormat getSwapChainImageFormat() { return mSwapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return mSwapChainExtent; }

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	void createSwapChain();
	void createImageViews();
	void createFramebuffers(VkRenderPass renderPass);
	void cleanupSwapChain();

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
	VkDevice mDevice = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	GLFWwindow* mWindow=nullptr;
	VkSurfaceKHR mSurface = VK_NULL_HANDLE;

	VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> mSwapChainImages{};
	std::vector<VkImageView> mSwapChainImageViews{};
	std::vector<VkFramebuffer> mSwapChainFramebuffers{};
	VkFormat mSwapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D mSwapChainExtent = { 0, 0 };
};
