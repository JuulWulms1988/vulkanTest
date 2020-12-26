#pragma once
#include "vulkan/vulkan.h"
#include "..\Base\\Device.hpp"

namespace vkMethods {


	namespace syncUpdateCompute {
		class base {
		public:
			vkMethods::buffer atomicBuffer;

			VkSemaphore perFrameSema;
			std::vector<VkSemaphore> perImSema;
			std::vector<VkCommandBuffer> perFrameCommandBuffers;

			virtual void cleanup() {
				vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
				vkDestroyPipelineLayout(device, perFramePushPipelineLayout, nullptr);
			}
			virtual void init() {
				initPerFramePush();
			}
			virtual void initPerFramePush() {
			

				VkSemaphoreCreateInfo semaphoreCreateInfo = vkMethods::initializers::semaphoreCreateInfo();
				vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &perFrameSema);
				perImSema.resize(swapChainImageCount);
				for (uint32_t i = 0; i < swapChainImageCount; i++) vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, perImSema.data() + i);
			
				createPerFrameCommandBuffers();
				buildPerFrameCommandBuffers();
			}
			
			virtual void updatePerFrameCommand(VkCommandBuffer commandBuffer, uint32_t count) = 0;
			

		protected:
			VkPipeline perFramePushPipeline;
			VkPipeline mainComputePipeline;
			VkPipelineLayout perFramePushPipelineLayout;
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorSet perFrameDescriptorSet;
			VkDescriptorSet mainComputeDescriptorSet[2];
		

			virtual void createPerFrameCommandBuffers() {
				perFrameCommandBuffers.resize(swapChainImageCount);

				VkCommandBufferAllocateInfo cmdBufAllocateInfo =
					vkMethods::initializers::commandBufferAllocateInfo(
						commandPool,
						VK_COMMAND_BUFFER_LEVEL_PRIMARY,
						swapChainImageCount);

				vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, perFrameCommandBuffers.data());
			}

		private:
			

			void buildPerFrameCommandBuffers() {
				uint32_t count = 0;
				for (VkCommandBuffer commandBuffer : perFrameCommandBuffers) {
					VkCommandBufferBeginInfo cmdBufInfo = vkMethods::initializers::commandBufferBeginInfo();

					vkBeginCommandBuffer(commandBuffer, &cmdBufInfo);

					updatePerFrameCommand(commandBuffer, count);

					vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, perFramePushPipeline);
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, perFramePushPipelineLayout, 0, 1, &perFrameDescriptorSet, 0, 0);

					
					vkCmdDispatch(commandBuffer, 1, 1, 1);

					

					vkEndCommandBuffer(commandBuffer);
					count++;
				}
			}
			
			
		};

	}
}