#pragma once

#include "vulkan/vulkan.h"
#include "Device.hpp"
#include "..\\Base\Info.hpp"

namespace vkMethods {
	struct buffer
		{
			VkBuffer buf = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			void create(
				VkDeviceSize bufferSize, 
				void* data, 
				vkMethods::info::VkQueueFamilySharingData sData, 
				VkQueue queue, 
				VkCommandPool comPool, 
				VkBufferUsageFlags bufFlags, 
				VkMemoryPropertyFlags memFlags) 
			{
				using namespace vkPointers;
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingBufferMemory;

				vkMethods::deviceS::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkMethods::info::VkQueueFamShareInfEx(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
				void* dat;
				vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &dat);
				memcpy(dat, data, (size_t)bufferSize);
				vkUnmapMemory(device, stagingBufferMemory);

				vkMethods::deviceS::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufFlags, sData, memFlags, buf, memory);
				
				deviceS::copyBuffer(stagingBuffer, buf, bufferSize, queue, comPool);
				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);
			}
			void update(VkDeviceSize size, void* data, VkQueue queue, VkCommandPool comPool, size_t offset) {
				using namespace vkPointers;
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingBufferMemory;

				vkMethods::deviceS::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkMethods::info::VkQueueFamShareInfEx(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
				void* dat;
				vkMapMemory(device, stagingBufferMemory, 0, size, 0, &dat);
				memcpy(dat, data, (size_t)size);
				vkUnmapMemory(device, stagingBufferMemory);

				deviceS::copyBuffer(stagingBuffer, buf, size, queue, comPool, offset);
				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);
			}
			void destroy()
			{
				if (buf)
				{
					vkDestroyBuffer(device, buf, nullptr);
				}
				if (memory)
				{
					vkFreeMemory(device, memory, nullptr);
				}
			}
		};
}