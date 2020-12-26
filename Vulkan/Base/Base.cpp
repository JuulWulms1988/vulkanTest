#include "Base.h"

#include <fstream>

static const int WIDTH = 800;
static const int HEIGHT = 600;



const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

namespace vkMethods::vkPointers {
	size_t currentFrame = 0;
	VkInstance vulkanInstance;
	VkImageView colorImageView;
	uint32_t imageIndex;
	uint8_t swapChainImageCount;
	VkDescriptorPool mainDescriptorPool;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSets[2];
	void* window;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	std::vector<VkImageView> swapChainImageViews;
	VkExtent2D swapChainExtent;
	VkSurfaceKHR surface;
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME, VK_KHR_MULTIVIEW_EXTENSION_NAME
	};
	VkQueue graphicsQueue;
	VkRenderPass renderPass;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkQueue presentQueue;
	VkQueue* computeQueue = VK_NULL_HANDLE;
	VkQueue* graphQueue = VK_NULL_HANDLE;
	VkQueue* transferQueue = VK_NULL_HANDLE;
	int VkComPoolCount_Graph = 0;
	int VkComPoolCount_Compute = 0;
	int VkComPoolCount_Trans = 0;
	VkCommandPool commandPool;
	VkCommandPool* computeCommandPool = VK_NULL_HANDLE;
	VkCommandPool* graphCommandPool = VK_NULL_HANDLE;
	VkCommandPool* transferCommandPool = VK_NULL_HANDLE;
	info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfComTrans;
	info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfGraphTrans;
	info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfGraphCom;
	info::VkQueueFamilySharingInfo<VkQueueFamily3Indices> VkQueueFamShareInfComTransGraph;
}

using namespace vkMethods::vkPointers;

void vulkanBase::run() {
	initWindow();
	initVulkan();
	//while (true);
	mainLoop();
	cleanup();
}

void vulkanBase::initWindow() {
	vkMethods::windowCls::initGlfw();
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer((GLFWwindow*)window, this);
	glfwSetFramebufferSizeCallback((GLFWwindow*)window, framebufferResizeCallback);
}

void vulkanBase::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<vulkanBase*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void vulkanBase::heapAllocF(vkMethods::info::VkBaseCreateInfo* vkBaseInfo) {
	char* p = (char*)(pHeapAllocBase = malloc(heapAllocBase =
		(vkBaseInfo->VkQueueCount_Graph +
			vkBaseInfo->VkQueueCount_Compute +
			vkBaseInfo->VkQueueCount_Trans)
		* sizeof(VkQueue) + (
			vkBaseInfo->VkComPoolCount_Compute +
			vkBaseInfo->VkComPoolCount_Trans +
			vkBaseInfo->VkComPoolCount_Graph)
		* sizeof(VkCommandPool)));
	if (vkBaseInfo->VkQueueCount_Graph)
		graphQueue = (VkQueue*)p, p += sizeof(VkQueue) * vkBaseInfo->VkQueueCount_Graph;
	if (vkBaseInfo->VkQueueCount_Compute)
		computeQueue = (VkQueue*)p, p += sizeof(VkQueue) * vkBaseInfo->VkQueueCount_Compute;
	if (vkBaseInfo->VkQueueCount_Trans)
		transferQueue = (VkQueue*)p, p += sizeof(VkQueue) * vkBaseInfo->VkQueueCount_Trans;
	if (VkComPoolCount_Graph = vkBaseInfo->VkComPoolCount_Graph)
		graphCommandPool = (VkCommandPool*)p, p += sizeof(VkCommandPool) * VkComPoolCount_Graph;
	if (VkComPoolCount_Trans = vkBaseInfo->VkComPoolCount_Trans)
		transferCommandPool = (VkCommandPool*)p, p += sizeof(VkCommandPool) * VkComPoolCount_Trans;
	if (VkComPoolCount_Compute = vkBaseInfo->VkComPoolCount_Compute)
		computeCommandPool = (VkCommandPool*)p/*, p += sizeof(VkCommandPool) * VkComPoolCount_Compute*/;
}

void vulkanBase::initVulkanBase(vkMethods::info::VkBaseCreateInfo* vkBaseInfo) {
	heapAllocF(vkBaseInfo);
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	vkMethods::deviceS::createLogicalDevice(vkBaseInfo);
	vkMethods::swapchain::createSwapChain(swapChain, swapChainImages, surface, swapChainExtent, swapChainImageFormat);
	vkMethods::swapchain::createImageViews(swapChainImageViews, swapChainImages, swapChainImageFormat);
	createRenderPass();
	createCommandPool(vkBaseInfo);
	//createColorResources();
	createDepthResources();
	//createFramebuffers();
	createCommandBuffers();
	createSyncObjects();
}

void vulkanBase::mainLoop() {
	while (!glfwWindowShouldClose((GLFWwindow*)window)) {
		glfwPollEvents();
		render();
	}

	vkDeviceWaitIdle(device);
}

void vulkanBase::cleanupSwapChain() {
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

}

void vulkanBase::cleanup() {
	vkDestroyDescriptorPool(device, mainDescriptorPool, nullptr);
	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	if (computeCommandPool && computeCommandPool != graphCommandPool && computeCommandPool != transferCommandPool && computeCommandPool != &commandPool)
		vkDestroyCommandPool(device, *computeCommandPool, nullptr);
	if (graphCommandPool && graphCommandPool != transferCommandPool && graphCommandPool != &commandPool)
		vkDestroyCommandPool(device, *graphCommandPool, nullptr);
	if (transferCommandPool && transferCommandPool != &commandPool)
		vkDestroyCommandPool(device, *transferCommandPool, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);


	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
	vkDestroyInstance(vulkanInstance, nullptr);

	glfwDestroyWindow((GLFWwindow*)window);

	glfwTerminate();
	delete[heapAllocBase] pHeapAllocBase;
}

namespace vkMethods {
	namespace swapchain {
		inline void getWindowsFrameB(int* width, int* height, bool waitEvents) {
			glfwGetFramebufferSize((GLFWwindow*)window, width, height);
			if (waitEvents) glfwWaitEvents();
		}
	}

}


void vulkanBase::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);
	reCreate();
}

void vulkanBase::reCreate() {
	cleanupSwapChain();
	vkMethods::swapchain::createSwapChain(swapChain, swapChainImages, surface, swapChainExtent, swapChainImageFormat);
	vkMethods::swapchain::createImageViews(swapChainImageViews, swapChainImages, swapChainImageFormat);
	createRenderPass();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createCommandBuffers();
}

void vulkanBase::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void vulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void vulkanBase::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void vulkanBase::createSurface() {
	if (glfwCreateWindowSurface(vulkanInstance, (GLFWwindow*)window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void vulkanBase::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}


void vulkanBase::createRenderPass() {
	return;
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = enableMultiView ? VK_SAMPLE_COUNT_1_BIT : msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = enableMultiView ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	vkMethods::deviceS::getSupportedDepthFormat(physicalDevice, &depthAttachment.format);
	depthAttachment.samples = enableMultiView ? VK_SAMPLE_COUNT_1_BIT : msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if (!enableMultiView) subpass.pResolveAttachments = &colorAttachmentResolveRef;

	/*
	
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		*/
	VkSubpassDependency dependency[2] = { {}, {} };
		dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency[0].dstSubpass = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
		dependency[1].srcSubpass = 0;
		dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(enableMultiView ? 2 : 3);
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void vulkanBase::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			swapChainImageViews[i],
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(enableMultiView ? 2 : 3);
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void vulkanBase::createCommandPool(vkMethods::info::VkBaseCreateInfo* vkBaseInfo ) {
	vkMethods::queueFamily::QueueFamilyIndices queueFamilyIndices = vkMethods::queueFamily::findQueueFamilies(physicalDevice, surface);
	vkCreateCommandPool(device, &vkMethods::initializers::commandPoolCreateInfo(vkBaseInfo->graphQueueFlags, queueFamilyIndices.graphicsFamily.index.value()), nullptr, &commandPool);
	auto f = [&](int v, uint32_t i, VkCommandPoolCreateFlags* f, VkCommandPool* p) {
		for (int t = 0; t < v; t++) vkCreateCommandPool(device, &vkMethods::initializers::commandPoolCreateInfo(f[t], i), nullptr, p + t);
	};
	if (vkBaseInfo->VkComPoolCount_Trans)
		f(vkBaseInfo->VkComPoolCount_Trans, queueFamilyIndices.transferFamily.index.value(), vkBaseInfo->transQueuesFlags, transferCommandPool);
	if (vkBaseInfo->VkComPoolCount_Compute)
		f(vkBaseInfo->VkComPoolCount_Compute, queueFamilyIndices.computeFamily.index.value(), vkBaseInfo->computeQueuesFlags, computeCommandPool);
	if (vkBaseInfo->VkComPoolCount_Graph)
		f(vkBaseInfo->VkComPoolCount_Graph, queueFamilyIndices.graphicsFamily.index.value(), vkBaseInfo->graphQueuesFlags, graphCommandPool);
}

void vulkanBase::createColorResources() {
	VkFormat colorFormat = swapChainImageFormat;

	vkMethods::deviceS::createImage(swapChainExtent.width, swapChainExtent.height, 1, enableMultiView ? VK_SAMPLE_COUNT_1_BIT : msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, vkMethods::info::VkQueueFamShareInfEx(), 1);
	colorImageView = vkMethods::swapchain::createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
}

void vulkanBase::createDepthResources() {
	VkFormat depthFormat;// = vkMethods::deviceS//findDepthFormat();
	vkMethods::deviceS::getSupportedDepthFormat(physicalDevice, &depthFormat);
	vkMethods::deviceS::createImage(swapChainExtent.width, swapChainExtent.height, 1, enableMultiView ? VK_SAMPLE_COUNT_1_BIT : msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, vkMethods::info::VkQueueFamShareInfEx(), 1);
	depthImageView = vkMethods::swapchain::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1, 1);
}

VkFormat vulkanBase::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat vulkanBase::findDepthFormat() {
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool vulkanBase::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkSampleCountFlagBits vulkanBase::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

inline void vulkanBase::createCommandBuffers() {
	commandBuffers.resize(swapChainImageCount * 2);
	if (vkAllocateCommandBuffers(device, &(vkMethods::initializers::commandBufferAllocateInfo(graphCommandPool[0], VK_COMMAND_BUFFER_LEVEL_PRIMARY, (uint32_t)commandBuffers.size())), commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void vulkanBase::createSyncObjects() {
	imageAvailableSemaphores.resize(swapChainImages.size() * 2);
	renderFinishedSemaphores.resize(swapChainImages.size() * 2);
	inFlightFences.resize(swapChainImages.size() + 1);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	std::cout << "eejSize: " << swapChainImages.size() << '\n';
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (i < swapChainImages.size() && vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
	if (swapChainImages.size() > MAX_FRAMES_IN_FLIGHT)
		for (size_t i = MAX_FRAMES_IN_FLIGHT; i < swapChainImages.size() * 2; i++)
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]),
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]),
			i <= swapChainImages.size() && vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
}

int yoyoFFBIJHOUDE = 0;



void vulkanBase::prepareDraw(VkResult(*funcNext)(uint32_t), uint32_t size) {
	submitInfo = VkSubmitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	static uint32_t ff = false;
	if (!ff && !ff++) {
		uint32_t g = imageIndex, c = (uint32_t)currentFrame;
		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, VK_NULL_HANDLE, inFlightFences[currentFrame], &imageIndex);
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		imageIndex = g, currentFrame = (size_t)c;
	}
	for (uint32_t i = 0; i < size; i++)
		if (funcNext(i) == VK_ERROR_OUT_OF_DATE_KHR) recreateSwapChain();
}

VkSwapchainKHR ffSWAPPP;

void vulkanBase::finishDraw() {
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 0;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

	VkSwapchainKHR swapChains[] = { ffSWAPPP };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;


	presentInfo.pImageIndices = &imageIndex;
	
	vkQueueWaitIdle(currentFrame ? presentQueue : graphicsQueue);







	switch (vkQueuePresentKHR(currentFrame ? graphicsQueue : presentQueue, &presentInfo)) {
	case VK_ERROR_OUT_OF_DATE_KHR: break;
	case VK_SUBOPTIMAL_KHR: break;
	case VK_SUCCESS: break;
	default: throw std::runtime_error("failed to present swap chain image!");
	}
	if (framebufferResized) framebufferResized = false, recreateSwapChain();
	
	
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


bool vulkanBase::isDeviceSuitable(VkPhysicalDevice device) {
	vkMethods::queueFamily::QueueFamilyIndices indices = vkMethods::queueFamily::findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		vkMethods::swapchain::SwapChainSupportDetails swapChainSupport = vkMethods::swapchain::querySwapChainSupport(surface, device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool vulkanBase::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

std::vector<const char*> vulkanBase::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool vulkanBase::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanBase::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}