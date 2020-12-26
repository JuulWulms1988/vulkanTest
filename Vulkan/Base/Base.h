#pragma once

#include "..\\Window\Window.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

#include "Tools.h"
#include "Initializers.hpp"
#include "Swapchain.hpp"
#include "Model.hpp"

extern const bool enableMultiView;

extern const int MAX_FRAMES_IN_FLIGHT;

extern const bool enableValidationLayers;

extern const std::vector<const char*> validationLayers;

extern const int WIDTH;
extern const int HEIGHT;

class vulkanBase {
public:
	void run();
	virtual void reCreate();
	std::vector<VkCommandBuffer> commandBuffers;

protected:
	virtual void render() = 0;
	void prepareDraw(VkResult(*funcNext)(uint32_t), uint32_t size);
	void finishDraw();


	uint8_t subMitSemaFirst = false;
	VkSubmitInfo submitInfo;
	VkPresentInfoKHR presentInfo;



	
	virtual void initVulkan() = 0;

	void initVulkanBase(vkMethods::info::VkBaseCreateInfo* vkBaseInfo);
	std::vector<VkFence> inFlightFences;


	virtual void cleanup();

	VkFormat findDepthFormat();
	std::vector<VkSemaphore> renderFinishedSemaphores;

private:

	// Contains the instanced data

	VkDebugUtilsMessengerEXT debugMessenger;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	

	std::vector<VkFence> imagesInFlight;

	VkSemaphore signalSemaphores[1];
	VkSemaphore waitSemaphores[1];
	VkPipelineStageFlags waitStages[3];

	bool framebufferResized = false;
	size_t heapAllocBase;
	void* pHeapAllocBase;

	void initWindow();
	void heapAllocF(vkMethods::info::VkBaseCreateInfo* vkBaseInfo);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void mainLoop();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createInstance();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createRenderPass();

	void createFramebuffers();

	void createCommandPool(vkMethods::info::VkBaseCreateInfo* vkBaseInfo);

	void createColorResources();

	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	bool hasStencilComponent(VkFormat format);

	VkSampleCountFlagBits getMaxUsableSampleCount();

	void createCommandBuffers();

	void createSyncObjects();

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();

	bool checkValidationLayerSupport();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};