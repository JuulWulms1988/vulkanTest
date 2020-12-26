#pragma once
#include "vulkan/vulkan.h"
#include "..\Base\\Device.hpp"

namespace vkMethods {
	namespace vkPointers {
		extern std::vector<VkFramebuffer> swapChainFramebuffers;
		extern VkSampleCountFlagBits msaaSamples;
		extern VkImageView colorImageView;
		extern VkRenderPass renderPass;
	}


	struct MultiviewPass {
		struct FrameBufferAttachment {
			VkImage image;
			VkDeviceMemory memory;
			VkImageView view;
		} color[2], depth[2], resolve[2];
		VkFramebuffer frameBuffer[2];
		VkRenderPass renderPass;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		VkSemaphore semaphore[8];
		VkCommandBuffer commandBuffers[6];
		vkMethods::buffer uniformBuffer;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSet descriptorSet[2];
		VkPipelineShaderStageCreateInfo shaderStages[2];


		enum activeViewsFlags { 
			ACTIVE_VIEWS_NONE, 
			ACTIVE_VIEW_SINGLE_BIT, 
			ACTIVE_VIEW_MULTI_BIT = 2,
		} activeViews = ACTIVE_VIEW_MULTI_BIT;
		enum pipelineViewType {
			PIPELINE_SINGLE,
			PIPELINE_MULTI
		};

		struct UniformBufferObject {
			alignas(16) glm::mat4 proj[3];
			uint32_t t = 0;
			uint32_t o = 0;
		} generateUbo() {
			if (activeViews == ACTIVE_VIEWS_NONE) return {};
			UniformBufferObject ubo = {};
			alignas(16) glm::mat4
				x = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f),
				y = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			x[1][1] *= -1;
			x *= y;

			if (activeViews & ACTIVE_VIEW_SINGLE_BIT) ubo.t++, ubo.o = 2, ubo.proj[2] = x;
			if (activeViews & ACTIVE_VIEW_MULTI_BIT) ubo.t += 2, ubo.o = 0, ubo.proj[0] = ubo.proj[1] = x;
			return ubo;
		};

		void updateUniformBuffers() {
			UniformBufferObject ubo = generateUbo();
			uniformBuffer.update(
				ubo.t * 16, 
				&ubo + ubo.o * 16, 
				transferQueue[0], 
				transferCommandPool[0], 
				ubo.o * 16
			);
		}

		void prepare(VkFormat depthFormat)
		{
			{
				UniformBufferObject ubo = generateUbo();
				uniformBuffer.create(
					sizeof(UniformBufferObject),
					&ubo,
					VkQueueFamShareInfComTransGraph.sData(),
					transferQueue[0],
					transferCommandPool[0],
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);
			}





			VkSamplerCreateInfo samplerCI = vkMethods::initializers::samplerCreateInfo();
			samplerCI.magFilter = VK_FILTER_NEAREST;
			samplerCI.minFilter = VK_FILTER_NEAREST;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.addressModeV = samplerCI.addressModeU;
			samplerCI.addressModeW = samplerCI.addressModeU;
			samplerCI.mipLodBias = 0.0f;
			samplerCI.maxAnisotropy = 1.0f;
			samplerCI.minLod = 0.0f;
			samplerCI.maxLod = 1.0f;
			samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;



			for (uint32_t i = 0; i < 2; i++)
			{



				deviceS::createImage(
					swapChainExtent.width,
					swapChainExtent.height,
					1,
					msaaSamples,
					depthFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					depth[i].image,
					depth[i].memory,
					vkMethods::info::VkQueueFamShareInfEx(),
					2,
					true
				);

				depth[i].view = swapchain::createImageView(
					depth[i].image, depthFormat,
					VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
					1,
					2
				);

				deviceS::createImage(
					swapChainExtent.width,
					swapChainExtent.height,
					1,
					msaaSamples,
					swapChainImageFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					color[i].image,
					color[i].memory,
					vkMethods::info::VkQueueFamShareInfEx(),
					2,
					true
				);

				



				color[i].view = swapchain::createImageView(
					color[i].image, swapChainImageFormat,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1,
					2
				);







				deviceS::createImage(
					swapChainExtent.width,
					swapChainExtent.height,
					1,
					VK_SAMPLE_COUNT_1_BIT,
					VK_FORMAT_B8G8R8A8_UNORM,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					resolve[i].image,
					resolve[i].memory,
					vkMethods::info::VkQueueFamShareInfEx(),
					2,
					true
				);

				resolve[i].view = swapchain::createImageView(
					resolve[i].image, VK_FORMAT_B8G8R8A8_UNORM,
					VK_IMAGE_ASPECT_COLOR_BIT,
					1,
					2
				);
			}

			vkCreateSampler(device, &samplerCI, nullptr, &sampler);


			descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptor.imageView = resolve[0].view;
			descriptor.sampler = sampler;



			for (uint32_t I = 0; I < 8; I++) vkCreateSemaphore(device, &vkMethods::initializers::semaphoreCreateInfo(), nullptr, semaphore + I);



			// Create sampler to sample from the attachment in the fragment shader


			// Fill a descriptor for later use in a descriptor set 

			

			/*
				Renderpass
			*/
			{
				std::array<VkAttachmentDescription, 3> attachments = {};
				// Color attachment
				attachments[0].format = swapChainImageFormat;
				attachments[0].samples = msaaSamples;
				attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				// Depth attachment
				attachments[1].format = depthFormat;
				attachments[1].samples = msaaSamples;
				attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				VkAttachmentDescription colorAttachmentResolve = {};
				attachments[2].format = swapChainImageFormat;
				attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
				attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[2].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

				VkAttachmentReference colorReference = {};
				colorReference.attachment = 0;
				colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkAttachmentReference depthReference = {};
				depthReference.attachment = 1;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				VkAttachmentReference colorAttachmentResolveRef = {};
				colorAttachmentResolveRef.attachment = 2;
				colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpassDescription = {};
				subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpassDescription.colorAttachmentCount = 1;
				subpassDescription.pColorAttachments = &colorReference;
				subpassDescription.pDepthStencilAttachment = &depthReference;
				subpassDescription.pResolveAttachments = &colorAttachmentResolveRef;

				// Subpass dependencies for layout transitions
				VkSubpassDependency dependency[2] = { {}, {} };
				dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency[0].dstSubpass = 0;
				dependency[0].srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				dependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				//dependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				dependency[1].srcSubpass = 0;
				dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				//dependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				
				VkRenderPassCreateInfo renderPassCI{};
				renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderPassCI.pAttachments = attachments.data();
				renderPassCI.subpassCount = 1;
				renderPassCI.pSubpasses = &subpassDescription;
				renderPassCI.dependencyCount = static_cast<uint32_t>(2);
				renderPassCI.pDependencies = dependency;

				/*
					Setup multiview info for the renderpass
				*/

				/*
					Bit mask that specifies which view rendering is broadcast to
					0011 = Broadcast to first and second view (layer)
				*/
				const uint32_t viewMask = 0b00000011;

				/*
					Bit mask that specifices correlation between views
					An implementation may use this for optimizations (concurrent render)
				*/
				const uint32_t correlationMask = 0b00000011;

				VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
				renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
				renderPassMultiviewCI.subpassCount = 1;
				renderPassMultiviewCI.pViewMasks = &viewMask;
				renderPassMultiviewCI.correlationMaskCount = 1;
				renderPassMultiviewCI.pCorrelationMasks = &correlationMask;

				renderPassCI.pNext = &renderPassMultiviewCI;

				vkCreateRenderPass(device, &renderPassCI, nullptr, &renderPass);
			}

			/*
				Framebuffer
			*/

			VkImageView attachments[3];

		

			
				
				for (uint32_t i = 0; i < 2; i++)
				{
					attachments[0] = color[i].view;
					attachments[1] = depth[i].view;
					attachments[2] = resolve[i].view;

				VkFramebufferCreateInfo framebufferCI = vkMethods::initializers::framebufferCreateInfo();
				framebufferCI.renderPass = renderPass;
				framebufferCI.attachmentCount = 3;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = swapChainExtent.width;
				framebufferCI.height = swapChainExtent.height;
				framebufferCI.layers = 1;
				vkCreateFramebuffer(device, &framebufferCI, nullptr, frameBuffer + i);

			}
		}
		void createCommandBuffers() {
			if (vkAllocateCommandBuffers(device, &vkMethods::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 6), commandBuffers) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
		}

		void buildCommandBuffers(VkPipeline* pPipeline) {
			return;
			VkCommandBufferBeginInfo cmdBufInfo = vkMethods::initializers::commandBufferBeginInfo();

			VkClearValue clearValues[2];
			clearValues[0].color = { 255.0f, 0.0f, 255.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = vkMethods::initializers::renderPassBeginInfo();
			renderPassBeginInfo.renderPass = vkPointers::renderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
			renderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues = clearValues;

			for (int32_t i = 0; i < 6; ++i) {
				renderPassBeginInfo.framebuffer = vkPointers::swapChainFramebuffers[i % 3];
				vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);
				vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline[0]);
				vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet + (i > 2), 0, nullptr);
			

				// Left eye

				vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline[1]);
				// Right eye
			
			


				vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

				vkCmdEndRenderPass(commandBuffers[i]);
				vkEndCommandBuffer(commandBuffers[i]);
			}
		}

		void createPipeline(pipelineViewType viewType, VkPipeline* pPipeline, VkRenderPass renderPass) {
			VkViewport viewport;
			VkRect2D scissor;
			VkGraphicsPipelineCreateInfo pipelineInfo = vkMethods::initializers::pipelineCreateInfo(VK_NULL_HANDLE, renderPass, 0);
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			VkPipelineInputAssemblyStateCreateInfo iAcrI = vkMethods::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			pipelineInfo.pInputAssemblyState = &iAcrI;
			VkPipelineViewportStateCreateInfo pVsI = vkMethods::initializers::pipelineViewportStateCreateInfo(1, 1, &viewport, &scissor);
			pipelineInfo.pViewportState = &pVsI;
			VkPipelineRasterizationStateCreateInfo pRsI = vkMethods::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
			pipelineInfo.pRasterizationState = &pRsI;
			VkPipelineMultisampleStateCreateInfo pMsSI = vkMethods::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
			pipelineInfo.pMultisampleState = &pMsSI;
			VkPipelineDepthStencilStateCreateInfo pDsI = vkMethods::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
			pipelineInfo.pDepthStencilState = &pDsI;
			VkPipelineColorBlendAttachmentState pCbAs = vkMethods::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
			VkPipelineColorBlendStateCreateInfo pCbSi = vkMethods::initializers::pipelineColorBlendStateCreateInfo(1, &pCbAs);
			pipelineInfo.pColorBlendState = &pCbSi;
			pipelineInfo.subpass = 0;
			
			float multiviewArrayLayer = 0.0f;

			VkSpecializationMapEntry specializationMapEntry{ 0, 0, sizeof(float) };

			VkSpecializationInfo specializationInfo{};
			specializationInfo.dataSize = sizeof(float);
			specializationInfo.mapEntryCount = 1;
			specializationInfo.pMapEntries = &specializationMapEntry;
			specializationInfo.pData = &multiviewArrayLayer;

			auto func = [&](uint32_t i) {
				VkPipeline OUT;
				shaderStages[1].pSpecializationInfo = &specializationInfo;
				multiviewArrayLayer = (float)i;
				VkPipelineVertexInputStateCreateInfo emptyInputState = vkMethods::initializers::pipelineVertexInputStateCreateInfo();
				pipelineInfo.pVertexInputState = &emptyInputState;
				vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &OUT);
				return OUT;
			};

			viewType ?
				(viewport = vkMethods::initializers::viewport((float)swapChainExtent.width / 2.0f, (float)swapChainExtent.height, 0.0f, 1.0f),
				scissor = vkMethods::initializers::rect2D(swapChainExtent.width / 2, swapChainExtent.height, 0, 0),
				pPipeline[0] = func(0),
				viewport.x = (float)swapChainExtent.width / 2,
				scissor.offset.x = swapChainExtent.width / 2,
				pPipeline[1] = func(1))
				:
				(viewport = vkMethods::initializers::viewport((float)swapChainExtent.width, (float)swapChainExtent.height, 0.0f, 1.0f),
				scissor = vkMethods::initializers::rect2D(swapChainExtent, 0, 0),
				*pPipeline = func(2));
		}

		void createDescriptor() {
			
			vkCreateDescriptorPool(
				device,
				&vkMethods::initializers::descriptorPoolCreateInfo(
					1,
					&vkMethods::initializers::descriptorPoolSize(
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						2
					), 2
				), nullptr,
				&descriptorPool
			);
			
			vkCreateDescriptorSetLayout(
				device,
				&vkMethods::initializers::descriptorSetLayoutCreateInfo(
					&vkMethods::initializers::descriptorSetLayoutBinding(
						VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						VK_SHADER_STAGE_FRAGMENT_BIT,
						0), 1
				), nullptr,
				&descriptorSetLayout
			);

			vkCreatePipelineLayout(device, &vkMethods::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1), nullptr, &pipelineLayout);

			VkDescriptorSetLayout layout[2]{ descriptorSetLayout, descriptorSetLayout };

			vkAllocateDescriptorSets(
				device,
				&vkMethods::initializers::descriptorSetAllocateInfo(
					descriptorPool,
					layout,
					2
				), descriptorSet
			);



			return;



			for (uint32_t i = 0; i < 2; i++) vkUpdateDescriptorSets(
				device,
				1,
				&vkMethods::initializers::writeDescriptorSet(
					descriptorSet[i],
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					0,
					&descriptor 
				), 0,
				nullptr
			);			
		}
	};
}