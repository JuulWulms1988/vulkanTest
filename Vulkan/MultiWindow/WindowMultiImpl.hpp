#pragma once

#include "..\\Multiview\BlitPass.hpp"
#include "..\\Window\WindowSwapChain.hpp"

extern VkSwapchainKHR ffSWAPPP;

namespace vkMethods {
	class windowMultiImpCls : public multiView::blitPassCls, public windowSwapChainCls {
	public:
		class uploadCommands {
		public:
			
			struct args {
				windowMultiImpCls** pWindows; 
				uint32_t windowCount; 
				uint32_t* pImageIndex; 
				VkImageMemoryBarrier* ptr;
				VkCommandBuffer commandBuffer;
				uint32_t currentFrame;
			} *ar;
			uploadCommands(args arg) {
				ar = &arg;

				for (uint32_t i = 0, m = arg.windowCount; i < m; i++) {
					if (i) {
						arg.ptr[i] = *arg.ptr;
						arg.ptr[i].image = arg.pWindows[i]->swapChainImages[arg.pImageIndex[i]];
						continue;
					}
					
					VkImageMemoryBarrier& barrier = arg.ptr[i] = VkImageMemoryBarrier{};
					
					barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					barrier.image = arg.pWindows[0]->swapChainImages[arg.pImageIndex[i]];
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.subresourceRange.baseMipLevel = 0;
					barrier.subresourceRange.levelCount = 1;
					barrier.subresourceRange.baseArrayLayer = 0;
					barrier.subresourceRange.layerCount = 1;
				}
				

				vkCmdPipelineBarrier(
					arg.commandBuffer,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 0,
					nullptr,
					0, nullptr,
					arg.windowCount, arg.ptr
				);



			}
			~uploadCommands() {

				for (uint32_t i = 0, m = ar->windowCount; i < m; i++) {
					vkMethods::multiView::blitPassCls::commandBufferBuildInfo buildInfo =
						blitPassCls::commandBufferBuildInfo{ ar->commandBuffer,
						multiViewOb.resolve[ar->currentFrame].image,
						ar->pWindows[i]->windowSwapChainCls::swapChainImages[ar->pImageIndex[i]],
						ar->pWindows[i]->windowSwapChainCls::swapChainExtent,
						vkPointers::swapChainExtent
					};
					ar->pWindows[i]->blitPassCls::buildCommandBuffers(buildInfo);
					VkImageMemoryBarrier& barrier = ar->ptr[i];
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				}
				vkCmdPipelineBarrier(
					ar->commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
					0, nullptr,
					ar->windowCount, ar->ptr
				);
			}
		};

		void init() {
			std::cout << "HEEJ\n"; while (true);
			windowSwapChainCls::init();
			ffSWAPPP = windowSwapChainCls::swapChain;
			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			std::cout << "eejSize: " << swapChainImages.size() << '\n';
			for (size_t i = 0; i < 2; i++)
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);


		}

		uint32_t imageIndex;

		
		VkResult acquireNext(uint32_t currentFrame) {
			vkGetFenceStatus(device, inFlightFences[currentFrame]);
			vkResetFences(device, 1, &inFlightFences[currentFrame]);
			

			return vkAcquireNextImageKHR(device, windowSwapChainCls::swapChain, UINT64_MAX, VK_NULL_HANDLE, inFlightFences[currentFrame], &imageIndex);
		}


	private:
		VkFence inFlightFences[2];
		
	};

}