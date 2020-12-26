#pragma once
#include "vulkan/vulkan.h"
#include "SyncUpdateCompute.hpp"
#include "..\\Base\Tools.h"


namespace vkMethods {
	namespace vkPointers {
		extern syncUpdateCompute::base& syncUpdateComputeOb;
	}

	namespace syncUpdateCompute {
		class app : public base {
		public:
			enum offsetsEnum { OFFSET_PER_FRAME_OB = 64, OFFSET_TOTAL = 192, OFFSET_COUNT = 1 };

			void cleanup() {
				base::cleanup();
			}
			
			struct updateDataStr {
				alignas (16) glm::mat4 perFrameData;
			};
			std::vector<updateDataStr> updateData;

		private:
			void init() {
				{
					uint32_t dat[OFFSET_TOTAL + OFFSET_COUNT * 24]{ 0 };
					atomicBuffer.create(
						OFFSET_TOTAL + OFFSET_COUNT * 96,
						&dat,
						VkQueueFamShareInfGraphCom.sData(),
						graphicsQueue, commandPool,
						VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
					);
				}
				base::init();
			}



			void createPerFrameCommandBuffers() {
				updateData.resize(swapChainImageCount);
				for (uint32_t i = 0; i < swapChainImageCount; i++) updateData[i].perFrameData = glm::mat4(1.0f);
				base::createPerFrameCommandBuffers();
			}

			void initPerFramePush() {
				// Get a compute capable device queue


				// Create compute pipeline
				// Compute pipelines are created separate from graphics pipelines even if they use the same queue (family index)

				std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings = {
					// Binding 0: Instance input data buffer
					vkMethods::initializers::descriptorSetLayoutBinding(
						VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						VK_SHADER_STAGE_COMPUTE_BIT,
						0),
					vkMethods::initializers::descriptorSetLayoutBinding(
						VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						VK_SHADER_STAGE_COMPUTE_BIT,
						1)

					// Binding 1: Indirect draw command output buffer (input)

					// Binding 2: Uniform buffer with global matrices (input)

				};

				VkDescriptorSetLayoutCreateInfo descriptorLayout =
					vkMethods::initializers::descriptorSetLayoutCreateInfo(
						setLayoutBindings.data(),
						static_cast<uint32_t>(setLayoutBindings.size()));

				vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout);

				VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
					vkMethods::initializers::pipelineLayoutCreateInfo(
						&descriptorSetLayout,
						1);

				VkPushConstantRange pushConstantRange =
					vkMethods::initializers::pushConstantRange(
						VK_SHADER_STAGE_COMPUTE_BIT,
						sizeof(OFFSET_PER_FRAME_OB),
						0);

				pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
				pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

				vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &perFramePushPipelineLayout);

				{


					VkDescriptorSetAllocateInfo allocInfo =
						vkMethods::initializers::descriptorSetAllocateInfo(
							mainDescriptorPool,
							&descriptorSetLayout,
							1);

					vkAllocateDescriptorSets(device, &allocInfo, &perFrameDescriptorSet);

					VkDescriptorBufferInfo bufferInfo = {};
					bufferInfo.buffer = atomicBuffer.buf;
					bufferInfo.offset = 0;
					bufferInfo.range = OFFSET_COUNT * 32;

					VkDescriptorBufferInfo buffer2Info = {};
					buffer2Info.buffer = atomicBuffer.buf;
					buffer2Info.offset = OFFSET_COUNT * 96;
					buffer2Info.range = OFFSET_TOTAL;

					VkWriteDescriptorSet computeWriteDescriptorSets[2] =
					{
						vkMethods::initializers::writeDescriptorSet(
							perFrameDescriptorSet,
							VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
							0,
							&bufferInfo),
						vkMethods::initializers::writeDescriptorSet(
							perFrameDescriptorSet,
							VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
							1,
							&buffer2Info),
					};

					vkUpdateDescriptorSets(device, 2, computeWriteDescriptorSets, 0, NULL);

				}
				{
					VkDescriptorSetLayout layouts[2]{ descriptorSetLayout, descriptorSetLayout };
					VkDescriptorSetAllocateInfo allocInfo =
						vkMethods::initializers::descriptorSetAllocateInfo(
							mainDescriptorPool,
							layouts,
							2);

					vkAllocateDescriptorSets(device, &allocInfo, mainComputeDescriptorSet);

					VkDescriptorBufferInfo bufferInfo = {};
					bufferInfo.buffer = atomicBuffer.buf;
					bufferInfo.offset = 0;
					bufferInfo.range = OFFSET_COUNT * 32;
					for (uint32_t i = 0; i < 2; i++) {
						VkDescriptorBufferInfo buffer2Info = {};
						buffer2Info.buffer = atomicBuffer.buf;
						buffer2Info.offset = OFFSET_COUNT * (i ? 64 : 32);
						buffer2Info.range = OFFSET_COUNT * 32;

						VkWriteDescriptorSet computeWriteDescriptorSets[2] =
						{
							vkMethods::initializers::writeDescriptorSet(
								mainComputeDescriptorSet[i],
								VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
								0,
								&bufferInfo),
							vkMethods::initializers::writeDescriptorSet(
								mainComputeDescriptorSet[i],
								VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
								1,
								&buffer2Info),
						};

						vkUpdateDescriptorSets(device, 2, computeWriteDescriptorSets, 0, NULL);
					}
				}


				// Create pipeline		
				VkComputePipelineCreateInfo computePipelineCreateInfo[2]{
					vkMethods::initializers::computePipelineCreateInfo(perFramePushPipelineLayout),
					vkMethods::initializers::computePipelineCreateInfo(perFramePushPipelineLayout)
						};

			
				
			//	computePipelineCreateInfo.stage = loadShader(getAssetPath() + "shaders/computecullandlod/cull.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
				
				HELPERS_STRING({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' _CMA_ 'c' _CMA_ 'o' _CMA_ 'm' _CMA_ 'p' _CMA_ 'S' _CMA_ 'y' _CMA_ 'n' _CMA_ 'W'
					_CMA_ 'r' _CMA_ 'i' _CMA_ 't' _CMA_ 'e' _CMA_ 'M' _CMA_ '4' _CMA_ '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'v' _CMA_ '\0' }, { computePipelineCreateInfo[1].stage = tools::createShader(_H_Str, VK_SHADER_STAGE_COMPUTE_BIT); });

				HELPERS_STRING({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' _CMA_ 'c' _CMA_ 'o' _CMA_ 'm' _CMA_ 'p' _CMA_ 'M' _CMA_ 'a' _CMA_ 'i' _CMA_ 'n'
				_CMA_ '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'V' _CMA_ '\0' }, { computePipelineCreateInfo[0].stage = tools::createShader(_H_Str, VK_SHADER_STAGE_COMPUTE_BIT); });
				
				// off 0 
				VkPipeline setPipe[2];
				vkCreateComputePipelines(device, VK_NULL_HANDLE, 2, computePipelineCreateInfo, nullptr, setPipe);
				mainComputePipeline = setPipe[0];
				perFramePushPipeline = setPipe[1];
			

				vkDestroyShaderModule(device, computePipelineCreateInfo[0].stage.module, nullptr);
				vkDestroyShaderModule(device, computePipelineCreateInfo[1].stage.module, nullptr);
				// Separate command pool as queue family for compute may be different than graphics
				base::initPerFramePush();
				// Create a command buffer for compute operations
				

				// Build a single command buffer containing the compute dispatch commands
				
				
			}

			


			void updatePerFrameCommand(VkCommandBuffer commandBuffer, uint32_t count) {
				vkCmdPushConstants(
					commandBuffer,
					perFramePushPipelineLayout,
					VK_SHADER_STAGE_COMPUTE_BIT,
					0,
					sizeof(OFFSET_PER_FRAME_OB),
					&updateData.data()[count].perFrameData);
			}

		};

	}
}