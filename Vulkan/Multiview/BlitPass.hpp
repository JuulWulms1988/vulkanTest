#pragma once
#include "vulkan/vulkan.h"
#include "..\\Base\Device.hpp"
#include "..\\Base\Image.hpp"
#include "MultiView.hpp"


namespace vkMethods {
	namespace multiView {
		class blitPassCls {
		public:
			
			enum viewType {VIEWTYPE_SINGLE, VIEWTYPE_MULTI} viewT;

			struct commandBufferBuildInfo {
				VkCommandBuffer commandBuffer;
				VkImage srcImage;
				VkImage dstImage;
				VkExtent2D dstExtent;
				VkExtent2D srcExtent;

				bool compExt() { return dstExtent.width == srcExtent.width && dstExtent.height == srcExtent.height; }
			};

			void buildCommandBuffers(commandBufferBuildInfo buildInfo) {
				if (buildInfo.dstExtent.width / (viewT == VIEWTYPE_MULTI ? 2 : 1) == buildInfo.srcExtent.width && !(viewT == VIEWTYPE_MULTI && buildInfo.srcExtent.width % 2) && buildInfo.dstExtent.height == buildInfo.srcExtent.height) buildBlitCommandBuffer(buildInfo);//buildCopyCommandBuffer(buildInfo);
				else buildBlitCommandBuffer(buildInfo);
			}
			
		private:
			class imageSubRes {
			public:
				VkImageSubresourceLayers srcResource;
				VkImageSubresourceLayers dstResource;
				imageSubRes() {
					srcResource = {}, dstResource = {};
					srcResource.aspectMask = dstResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					srcResource.mipLevel = dstResource.mipLevel = 0;
					srcResource.baseArrayLayer = 0;
					dstResource.baseArrayLayer = 0;
					srcResource.layerCount = dstResource.layerCount = 1;
				}
			};

			void buildCopyCommandBuffer(commandBufferBuildInfo buildInfo) {
				//std::cout << "YOaaaaYO\n";
				imageSubRes subR;
				if (viewT == VIEWTYPE_MULTI) buildInfo.dstExtent.width /= 2;
				VkImageCopy copyRegion[2] = { {} };
				copyRegion[0].srcSubresource = subR.srcResource;
				copyRegion[0].dstSubresource = subR.dstResource;
				copyRegion[0].srcOffset = copyRegion[0].dstOffset = {0,0,0};
				copyRegion[0].extent.depth = 1;
				copyRegion[0].extent.height = buildInfo.dstExtent.height;
				copyRegion[0].extent.width = buildInfo.dstExtent.width;

				
				vkCmdCopyImage(
					buildInfo.commandBuffer,
					buildInfo.srcImage,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					buildInfo.dstImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					viewT == VIEWTYPE_MULTI ? (
						copyRegion[1] = copyRegion[0], copyRegion[1].srcSubresource.baseArrayLayer = 1, copyRegion[1].dstOffset.x = buildInfo.dstExtent.width,
						2) : (copyRegion[0].srcSubresource.baseArrayLayer = 2, 1),
					copyRegion
				);
			}

			void buildBlitCommandBuffer(commandBufferBuildInfo buildInfo) {
				imageSubRes subR;
				
				
				
				
				VkImageBlit copyRegion[2] = { {} };
				copyRegion[0].srcSubresource = subR.srcResource;
				copyRegion[0].dstSubresource = subR.dstResource;
				copyRegion[0].srcOffsets[0] = copyRegion[0].dstOffsets[0] = { 0,0,0 };
				copyRegion[0].srcOffsets[1].z = copyRegion[0].dstOffsets[1].z = 1;
				copyRegion[0].srcOffsets[1].x = buildInfo.srcExtent.width;
				copyRegion[0].srcOffsets[1].y = buildInfo.srcExtent.height;
				copyRegion[0].dstOffsets[1].x = buildInfo.dstExtent.width / (viewT == VIEWTYPE_MULTI ? 2 : 1);
				copyRegion[0].dstOffsets[1].y = buildInfo.dstExtent.height;




				
				vkCmdBlitImage(
					buildInfo.commandBuffer,
					buildInfo.srcImage,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					buildInfo.dstImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					viewT == VIEWTYPE_MULTI ? (
						copyRegion[1] = copyRegion[0], copyRegion[1].srcSubresource.baseArrayLayer = 1, copyRegion[1].dstOffsets[0].x = copyRegion[1].dstOffsets[1].x, copyRegion[1].dstOffsets[1].x = buildInfo.dstExtent.width,
						2) : (copyRegion[0].srcSubresource.baseArrayLayer = 2, 1),
					copyRegion,
					VK_FILTER_LINEAR
				);
				
				
			}
		};
	}
}