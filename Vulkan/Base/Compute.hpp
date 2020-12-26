#pragma once

#include "vulkan/vulkan.h"
#include "Tools.h"
#include "Info.hpp"

#include <vector>
#include <optional>

namespace vkMethods {
	namespace vkPointers {
		extern VkSurfaceKHR surface;
		extern VkPhysicalDevice physicalDevice;
		extern VkDevice device;
		typedef uint32_t VkQueueFamily2Indices[2];
		typedef uint32_t VkQueueFamily3Indices[3];
		extern info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfComTrans;
		extern info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfGraphTrans;
		extern info::VkQueueFamilySharingInfo<VkQueueFamily2Indices> VkQueueFamShareInfGraphCom;
		extern info::VkQueueFamilySharingInfo<VkQueueFamily3Indices> VkQueueFamShareInfComTransGraph;


	}

	namespace queueFamily {
		using namespace vkPointers;
		struct QueueFamilyIndices {
			struct {
				std::optional<uint32_t> index;
				int cnt;
			} graphicsFamily, presentFamily, transferFamily, computeFamily;
			bool isComplete() {
				return graphicsFamily.index.has_value() && presentFamily.index.has_value() && transferFamily.index.has_value() && computeFamily.index.has_value();
			}
		};

		inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR surface) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily.index = i, indices.graphicsFamily.cnt = queueFamily.queueCount;
				}

				else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
					indices.computeFamily.index = i, indices.computeFamily.cnt = queueFamily.queueCount;
				}
				else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					indices.transferFamily.index = i, indices.transferFamily.cnt = queueFamily.queueCount;
				}
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, surface, &presentSupport);

				if (presentSupport) {
					indices.presentFamily.index = i;
				}
				if (indices.isComplete()) {
					break;
				}

				i++;
			}
			if (indices.graphicsFamily.index.has_value()) {
				if (!indices.computeFamily.index.has_value()) indices.computeFamily = indices.graphicsFamily;
				if (!indices.transferFamily.index.has_value()) indices.transferFamily = indices.computeFamily;
			}
			return indices;
		}

		enum vulkanQueueFamIndex { VULKAN_QUEUE_FAM_INDEX_GRAPH, VULKAN_QUEUE_FAM_INDEX_TRANFER, VULKAN_QUEUE_FAM_INDEX_COMPUTE };

		inline VkCommandPool createCommandPool(vulkanQueueFamIndex x, VkCommandPoolCreateFlags y) {
			uint32_t index;
			{
				QueueFamilyIndices z = findQueueFamilies(physicalDevice, surface);
				switch (x) {
				case VULKAN_QUEUE_FAM_INDEX_TRANFER:
					index = z.transferFamily.index.value();
					break;
				case VULKAN_QUEUE_FAM_INDEX_COMPUTE:
					index = z.computeFamily.index.value();
					break;
				case VULKAN_QUEUE_FAM_INDEX_GRAPH:
					index = z.graphicsFamily.index.value();
					break;
				}
			}
			VkCommandPool z;
			vkCreateCommandPool(device, &vkMethods::initializers::commandPoolCreateInfo(y, index), nullptr, &z);
			return z;
		}

		inline void setFamily(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, float* queuePriority, QueueFamilyIndices& indices, int(*alloc)[4], int VkQueueCount_Trans, int VkQueueCount_Compute, int VkQueueCount_Graph)
		{
			int left[3]{ 0,0,0 };
			left[0] = indices.graphicsFamily.cnt - (indices.graphicsFamily.index == indices.presentFamily.index);
			left[0] > 1 + VkQueueCount_Graph ? left[0] -= 1 + VkQueueCount_Graph : left[0] = 0;
			if (indices.graphicsFamily.index == indices.computeFamily.index)
				left[0] > VkQueueCount_Compute ? left[0] -= VkQueueCount_Compute : left[0] = 0;
			else
				left[1] = indices.computeFamily.cnt > VkQueueCount_Compute ? indices.computeFamily.cnt - VkQueueCount_Compute : 0,
				indices.transferFamily.index == indices.computeFamily.index && left[1] > VkQueueCount_Trans ? left[1] -= VkQueueCount_Trans : left[1] = 0;

			if (indices.transferFamily.index == indices.graphicsFamily.index)
				left[0] > VkQueueCount_Trans ? left[0] -= VkQueueCount_Trans : left[0] = 0;
			else if (indices.transferFamily.index != indices.computeFamily.index)
				left[2] = indices.transferFamily.cnt > VkQueueCount_Trans ? indices.transferFamily.cnt - VkQueueCount_Trans : 0;

			int pres = indices.graphicsFamily.index == indices.presentFamily.index;

			if (indices.graphicsFamily.cnt != 1) {
				alloc[0][1] = 0 + pres;
				if (left[0]) alloc[1][1] = pres + VkQueueCount_Graph;
				else {
					int c = indices.graphicsFamily.cnt - pres;
					if (c && indices.graphicsFamily.index == indices.computeFamily.index && VkQueueCount_Compute) c--;
					if (c && indices.graphicsFamily.index == indices.transferFamily.index && VkQueueCount_Trans) c--;
					alloc[1][1] = c >= pres + VkQueueCount_Graph ? pres + VkQueueCount_Graph : VkQueueCount_Graph - (VkQueueCount_Graph - c);
				}
			}
			else alloc[1][1] = 1;

			if (VkQueueCount_Compute)
				if (indices.computeFamily.index == indices.graphicsFamily.index)
					if (indices.graphicsFamily.cnt < 2 + pres) alloc[0][2] = alloc[0][1], alloc[1][2] = 1;
					else if (alloc[0][2] = alloc[1][1] + pres, left[0]) alloc[1][2] = VkQueueCount_Compute;
					else {
						int c = indices.graphicsFamily.cnt - (pres + alloc[1][1]);
						if (indices.graphicsFamily.index == indices.transferFamily.index && VkQueueCount_Trans) c--;
						alloc[1][2] = c >= VkQueueCount_Compute ? VkQueueCount_Compute : VkQueueCount_Compute - (VkQueueCount_Compute - c);
					}
				else
					if (left[1]) alloc[1][2] = VkQueueCount_Compute;
					else {
						int c = indices.computeFamily.cnt;
						if (indices.computeFamily.index == indices.transferFamily.index && VkQueueCount_Trans) c--;
						alloc[1][2] = c >= VkQueueCount_Compute ? VkQueueCount_Compute : VkQueueCount_Compute - (VkQueueCount_Compute - c);
					}

			if (VkQueueCount_Trans)
				if (indices.transferFamily.index == indices.graphicsFamily.index)
					if (indices.graphicsFamily.cnt < 2 + pres) alloc[0][3] = alloc[0][1], alloc[1][3] = 1;
					else if (alloc[0][3] = VkQueueCount_Compute && indices.computeFamily.index == indices.graphicsFamily.index ? alloc[1][2] + alloc[0][2] : alloc[1][1] + pres, left[0]) alloc[1][3] = VkQueueCount_Trans;
					else {
						int c = indices.graphicsFamily.cnt - (pres + alloc[1][1]);
						if (indices.computeFamily.index == indices.graphicsFamily.index)
							alloc[1][3] = VkQueueCount_Compute >= c ? 1 : VkQueueCount_Trans - (VkQueueCount_Trans - (c - VkQueueCount_Compute));
						else alloc[1][3] = c >= VkQueueCount_Trans ? VkQueueCount_Trans : VkQueueCount_Trans - (VkQueueCount_Trans - c);
					}

				else if (indices.transferFamily.index == indices.computeFamily.index) {
					if (indices.computeFamily.cnt == 1) alloc[1][3] = 1;
					else if (alloc[0][3] = alloc[1][2], left[1]) alloc[1][3] = VkQueueCount_Trans;
					else {
						int c = indices.computeFamily.cnt - (alloc[1][2]);
						alloc[1][3] = c >= VkQueueCount_Trans ? VkQueueCount_Trans : VkQueueCount_Trans - (VkQueueCount_Trans - c);
					}
				}
				else alloc[1][3] = left[2] ? VkQueueCount_Trans : VkQueueCount_Trans - (VkQueueCount_Trans - indices.transferFamily.cnt);

			{
				uint32_t* a = VkQueueFamShareInfComTrans.indices;
				if (VkQueueCount_Compute && VkQueueCount_Trans) VkQueueFamShareInfComTrans.count = (a[0] = indices.computeFamily.index.value()) == (a[1] = indices.transferFamily.index.value()) ? 0 : 2;
				a = VkQueueFamShareInfGraphCom.indices;
				if (VkQueueCount_Compute) VkQueueFamShareInfGraphCom.count = (a[0] = indices.graphicsFamily.index.value()) == (a[1] = indices.computeFamily.index.value()) ? 0 : 2;
				a = VkQueueFamShareInfGraphTrans.indices;
				if (VkQueueCount_Trans) VkQueueFamShareInfGraphTrans.count = (a[0] = indices.graphicsFamily.index.value()) == (a[1] = indices.transferFamily.index.value()) ? 0 : 2;
				a = VkQueueFamShareInfComTransGraph.indices;
				VkQueueFamShareInfComTransGraph.count = (a[0] = indices.graphicsFamily.index.value()) == (a[1] = indices.computeFamily.index.value() || !VkQueueCount_Trans) ? 1 : 2;
				if (VkQueueCount_Trans && indices.graphicsFamily.index != indices.transferFamily.index && !(VkQueueCount_Compute && indices.computeFamily.index == indices.transferFamily.index))
					VkQueueFamShareInfComTransGraph.indices[VkQueueFamShareInfComTransGraph.count++] = indices.transferFamily.index.value();
			}

			auto qInf = [&](uint32_t IND, uint32_t CNT) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = IND;
				queueCreateInfo.queueCount = CNT;
				queueCreateInfo.pQueuePriorities = queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			};

			if (indices.presentFamily.index != indices.graphicsFamily.index)
				qInf(indices.presentFamily.index.value(), 1);

			qInf(indices.graphicsFamily.index.value(), indices.graphicsFamily.cnt - left[0]);

			if (indices.computeFamily.index != indices.graphicsFamily.index && (VkQueueCount_Compute || (VkQueueCount_Trans && indices.computeFamily.index == indices.transferFamily.index)))
				qInf(indices.computeFamily.index.value(), indices.computeFamily.cnt - left[1]);

			if (VkQueueCount_Trans && indices.transferFamily.index != indices.computeFamily.index && indices.transferFamily.index != indices.graphicsFamily.index)
				qInf(indices.transferFamily.index.value(), indices.transferFamily.cnt - left[2]);
		}
	}
}