#pragma once

#include "vulkan/vulkan.h"

#include <IOStream>

#include "QueueFamily.hpp"

#include "..\\..\\Helpers\Basic.h"

#include <stdexcept>

extern const bool enableValidationLayers;

extern const std::vector<const char*> validationLayers;

namespace vkMethods {
	namespace vkPointers {
		extern VkInstance vulkanInstance;
		extern uint8_t swapChainImageCount;
		extern std::vector<VkCommandBuffer> commandBuffers;
		extern uint32_t imageIndex;
		extern VkPhysicalDevice physicalDevice;
		extern VkDevice device;
		extern VkDescriptorSet descriptorSets[2];
		extern VkPipelineLayout pipelineLayout;
		extern VkSurfaceKHR surface;
		extern const std::vector<const char*> deviceExtensions;
		extern VkQueue graphicsQueue;
		extern VkQueue presentQueue;
		extern VkCommandPool commandPool;
		extern VkDescriptorPool mainDescriptorPool;
		extern VkQueue* computeQueue;
		extern VkQueue* graphQueue;
		extern VkQueue* transferQueue;
		extern int VkComPoolCount_Graph;
		extern int VkComPoolCount_Compute;
		extern int VkComPoolCount_Trans;
		
		extern VkCommandPool commandPool;
		extern VkCommandPool* computeCommandPool;
		extern VkCommandPool* graphCommandPool;
		extern VkCommandPool* transferCommandPool;
	}

	namespace deviceS {
		using namespace vkPointers;
		
		inline VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
		{
			// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
			std::vector<VkFormat> depthFormats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			for (auto& format : depthFormats)
			{
				VkFormatProperties formatProps;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
				// Format must support depth stencil attachment for optimal tiling
				if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					*depthFormat = format;
					return true;
				}
			}

			return false;
		}

		inline void createLogicalDevice(info::VkBaseCreateInfo* vkBaseInfo) {
			Helpers::basic::allocaFunction([](void* ptrI, void* ptr) {
				info::VkBaseCreateInfo* vkBaseInfo = (info::VkBaseCreateInfo*)ptrI;
				float* queuePriority = (float*)ptr;
				queueFamily::QueueFamilyIndices indices = queueFamily::findQueueFamilies(physicalDevice, surface);
				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

				int	alloc[2][4]{ {0,0,0,0}, {1,0,0,0} }, s = vkBaseInfo->VkQueueCount_Compute + vkBaseInfo->VkQueueCount_Graph + vkBaseInfo->VkQueueCount_Trans + 2;

				for (int t = 0; t < s; t++) queuePriority[t] = 1.0f;
				queueFamily::setFamily(queueCreateInfos, queuePriority, indices, alloc, vkBaseInfo->VkQueueCount_Trans, vkBaseInfo->VkQueueCount_Compute, vkBaseInfo->VkQueueCount_Graph);

				VkPhysicalDeviceFeatures deviceFeatures = {};
				deviceFeatures.samplerAnisotropy = VK_TRUE;

				VkDeviceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

				createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
				createInfo.pQueueCreateInfos = queueCreateInfos.data();

				createInfo.pEnabledFeatures = &deviceFeatures;

				createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
				createInfo.ppEnabledExtensionNames = deviceExtensions.data();

				if (enableValidationLayers) {
					createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();
				}
				else {
					createInfo.enabledLayerCount = 0;
				}

				if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
					throw std::runtime_error("failed to create logical device!");
				}

				vkGetDeviceQueue(device, indices.graphicsFamily.index.value(), alloc[0][1], &graphicsQueue);
				vkGetDeviceQueue(device, indices.presentFamily.index.value(), 0, &presentQueue);
				for (int t = 0; t < vkBaseInfo->VkQueueCount_Graph; t++) vkGetDeviceQueue(device, indices.graphicsFamily.index.value(), alloc[0][1] + (t + 1 % alloc[1][1]), graphQueue + t);
				for (int t = 0; t < vkBaseInfo->VkQueueCount_Compute; t++) vkGetDeviceQueue(device, indices.computeFamily.index.value(), alloc[0][2] + (t % alloc[1][2]), computeQueue + t);
				for (int t = 0; t < vkBaseInfo->VkQueueCount_Trans; t++) vkGetDeviceQueue(device, indices.transferFamily.index.value(), alloc[0][3] + (t % alloc[1][3]), transferQueue + t);
				}, vkBaseInfo, (vkBaseInfo->VkQueueCount_Compute + vkBaseInfo->VkQueueCount_Graph + vkBaseInfo->VkQueueCount_Trans + 2) * sizeof(float));
		}

		inline uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}

		inline void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, info::VkQueueFamilySharingData shareData, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = shareData.sMode;
			bufferInfo.pQueueFamilyIndices = shareData.ar;
			bufferInfo.queueFamilyIndexCount = shareData.count;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("failed to create buffer!");
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate buffer memory!");
			}

			vkBindBufferMemory(device, buffer, bufferMemory, 0);
		}

		inline void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, info::VkQueueFamilySharingData sData, uint32_t layerCount, bool dedicated = 0U) {
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = mipLevels;
			imageInfo.arrayLayers = layerCount;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = numSamples;
			imageInfo.sharingMode = sData.sMode;
			imageInfo.pQueueFamilyIndices = sData.ar;
			imageInfo.queueFamilyIndexCount = sData.count;

			VkDedicatedAllocationImageCreateInfoNV dedicatedImageInfo = { VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV };

			if (dedicated) {
				dedicatedImageInfo.dedicatedAllocation = VK_TRUE;
				imageInfo.pNext = &dedicatedImageInfo;
			}
			if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image!");
			}

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);


			VkDedicatedAllocationMemoryAllocateInfoNV dedicatedAllocationInfo = { VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV };
			if (dedicated)
			{
				dedicatedAllocationInfo.image = image;
				allocInfo.pNext = &dedicatedAllocationInfo;
			}
			if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate image memory!");
			}

			vkBindImageMemory(device, image, imageMemory, 0);
		}

		inline VkCommandBuffer beginSingleTimeCommands(VkCommandPool comPool) {
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = comPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			return commandBuffer;
		}

		inline void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool comPool) {
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);

			vkFreeCommandBuffers(device, comPool, 1, &commandBuffer);
		}

		inline void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue, VkCommandPool comPool, size_t dstOff = 0U, size_t srcOff = 0U) {
			VkCommandBuffer commandBuffer = vkMethods::deviceS::beginSingleTimeCommands(comPool);

			VkBufferCopy copyRegion = {};
			copyRegion.size = size;
			copyRegion.dstOffset = dstOff;
			copyRegion.srcOffset = srcOff;
			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			vkMethods::deviceS::endSingleTimeCommands(commandBuffer, queue, comPool);
		}
	}
}