#pragma once

#include "vulkan/vulkan.h"

namespace vkMethods::info {
	struct VkBaseCreateInfo {
		VkCommandPoolCreateFlags graphQueueFlags;
		int VkQueueCount_Trans = 0;
		int VkComPoolCount_Trans = 0;
		VkCommandPoolCreateFlags* transQueuesFlags;
		int VkQueueCount_Graph = 0;
		int VkComPoolCount_Graph = 0;
		VkCommandPoolCreateFlags* graphQueuesFlags;
		int VkQueueCount_Compute = 0;
		int VkComPoolCount_Compute = 0;
		VkCommandPoolCreateFlags* computeQueuesFlags;
	}; 

	struct VkQueueFamilySharingData {
		uint32_t count;
		uint32_t* ar;
		VkSharingMode sMode;
	};

	inline VkQueueFamilySharingData VkQueueFamShareInfEx() {
		return VkQueueFamilySharingData{ 0, nullptr, VK_SHARING_MODE_EXCLUSIVE };
	}

	template<class ar>
	struct VkQueueFamilySharingInfo{
		ar indices;
		uint32_t count = 0;
		VkQueueFamilySharingData sData() {
			return VkQueueFamilySharingData{ count, indices, count ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE };
		}
	};
}