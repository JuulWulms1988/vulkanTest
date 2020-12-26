#include "TileIndirectCompute0Implementation.h"

static const std::string TEXTURE_PATH = "textures/chalet.jpg";
static const std::string MODEL_PATH = "models/chalet.obj";

#include <thread>
#include <chrono>

namespace vkMethods {

	namespace vkPointers {
		extern size_t currentFrame;
	}

	namespace TileIndirectCompute0 {

		using namespace vkPointers;

		base& base::pTile_Main = CONSTR_TILEINDIRECTCOMPUTE0;

		void app::reCreate(void(*engineRecreate)(void*), void* pObj) {
			base::reCreate();
			engineRecreate(pObj);
			createGraphicsPipeline();
			pTile_Main.reCreateObj();
		}

		void app::reCreateObj() {
			createDescriptorPool();
			createDescriptorSets();
			base::reCreateObj();
		}

		void app::initVulkan() {
			createDescriptorSetLayout();
			createDescriptorPool();
			createGraphicsPipeline();
			pTile_Main.init();
		}

		void app::init() {
			vkMethods::tools::loadTexTest::load(TEXTURE_PATH, textureS);
			textureS.createTextureImageView();
			textureS.createTextureSampler();
			vkMethods::tools::loadModelTest(MODEL_PATH, vertices, indices);
			prepareInstanceData();
			modelS.vertices.create(sizeof(vertices[0]) * vertices.size(), vertices.data(), VkQueueFamShareInfComTransGraph.sData(), transferQueue[0], transferCommandPool[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			modelS.indices.create(sizeof(indices[0]) * indices.size(), indices.data(), VkQueueFamShareInfComTransGraph.sData(), transferQueue[0], transferCommandPool[0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			createUniformBuffers();
			createDescriptorSets();
			base::init();
		}

		void app::cleanup() {
			base::cleanup();
		}

		void app::cleanupObj() {
			base::cleanupObj();
		}

		void app::render() {
			base::render();
		}

		void app::renderObj() {
			updateUniformBuffers((uint32_t)currentFrame);
		}

		void app::createDescriptorSetLayout() {
			VkDescriptorSetLayoutBinding bindings[3]{
				vkMethods::initializers::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT,
					0),
				vkMethods::initializers::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT,
					1),
				vkMethods::initializers::descriptorSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					2),
			};
			VkDescriptorSetLayoutCreateInfo layoutInfo = vkMethods::initializers::descriptorSetLayoutCreateInfo(bindings, 3);

			if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}

		std::array<VkVertexInputAttributeDescription, 5> app::getAttributeDescriptions() {
			return std::array<VkVertexInputAttributeDescription, 5> {
				vkMethods::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkMethods::model::Vertex, pos)),
					vkMethods::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vkMethods::model::Vertex, color)),
					vkMethods::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(vkMethods::model::Vertex, texCoord)),
					vkMethods::initializers::vertexInputAttributeDescription(1, 3, VK_FORMAT_R32G32B32_SFLOAT, 0),
					vkMethods::initializers::vertexInputAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3)
			};
		}

		void app::createGraphicsPipeline() {
			VkPipelineShaderStageCreateInfo shaderStages[2]{};
			if (enableMultiView) {
				HELPERS_STRING_INS_C({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' }, 8, { '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'v' }, 4, {
						HELPERS_STRING_INS_F({'v' _CMA_ 'e' _CMA_ 'r' _CMA_ 't' _CMA_ 'M' }, 5, { shaderStages[0] = tools::createShader(_HH_Str, VK_SHADER_STAGE_VERTEX_BIT); })
						HELPERS_STRING_INS_F({'f' _CMA_ 'r' _CMA_ 'a' _CMA_ 'g' _CMA_ 'M' }, 5, { shaderStages[1] = tools::createShader(_HH_Str, VK_SHADER_STAGE_FRAGMENT_BIT); })
					});
			}
			else {
				HELPERS_STRING_INS_C({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' }, 8, { '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'v' }, 4, {
						HELPERS_STRING_INS_F({'v' _CMA_ 'e' _CMA_ 'r' _CMA_ 't' }, 4, { shaderStages[0] = tools::createShader(_HH_Str, VK_SHADER_STAGE_VERTEX_BIT); })
						HELPERS_STRING_INS_F({'f' _CMA_ 'r' _CMA_ 'a' _CMA_ 'g' }, 4, { shaderStages[1] = tools::createShader(_HH_Str, VK_SHADER_STAGE_FRAGMENT_BIT); })
					});
			}

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = vkMethods::initializers::pipelineVertexInputStateCreateInfo();
			std::vector<VkVertexInputBindingDescription> bindingDescription = {
				vkMethods::initializers::vertexBindingDescription(0, sizeof(vkMethods::model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
				vkMethods::initializers::vertexBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
			};

			auto attributeDescriptions = getAttributeDescriptions();

			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			VkViewport viewport = vkMethods::initializers::viewport((float)swapChainExtent.width, (float)swapChainExtent.height, 0.0f, 1.0f);
			VkRect2D scissor = vkMethods::initializers::rect2D(swapChainExtent, 0, 0);
			//vkMethods::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
			VkPipelineInputAssemblyStateCreateInfo inputAssembly = vkMethods::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			VkPipelineViewportStateCreateInfo viewportState = vkMethods::initializers::pipelineViewportStateCreateInfo(1, 1, &viewport, &scissor);
			VkPipelineRasterizationStateCreateInfo rasterizer = vkMethods::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
			VkPipelineMultisampleStateCreateInfo multisampling = vkMethods::initializers::pipelineMultisampleStateCreateInfo(msaaSamples, 0);
			VkPipelineDepthStencilStateCreateInfo depthStencil = vkMethods::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
			VkPipelineColorBlendAttachmentState colorBlendAttachment = vkMethods::initializers::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
			VkPipelineColorBlendStateCreateInfo colorBlending = vkMethods::initializers::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);
			VkDescriptorSetLayout layouts[2]{ descriptorSetLayout, descriptorSetLayout };
			VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkMethods::initializers::pipelineLayoutCreateInfo(layouts, 2);

			VkDynamicState dynamicStateEnables[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicState = vkMethods::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables, 2);
			if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}

			VkGraphicsPipelineCreateInfo pipelineInfo = vkMethods::initializers::pipelineCreateInfo(pipelineLayout, enableMultiView ? multiViewOb.renderPass : renderPass, 0);
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			//pipelineInfo.pDynamicState = &dynamicState;

			pipelineInfo.subpass = 0;

			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}




			vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
			vkDestroyShaderModule(device, shaderStages[1].module, nullptr);

			return;

			HELPERS_STRING_INS_C({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' }, 8, { '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'v' }, 4, {
					HELPERS_STRING_INS_F({'v' _CMA_ 'e' _CMA_ 'r' _CMA_ 't' _CMA_ 'G' }, 5, { multiViewOb.shaderStages[0] = tools::createShader(_HH_Str, VK_SHADER_STAGE_VERTEX_BIT); })
					HELPERS_STRING_INS_F({'f' _CMA_ 'r' _CMA_ 'a' _CMA_ 'g' _CMA_ 'G' }, 5, { multiViewOb.shaderStages[1] = tools::createShader(_HH_Str, VK_SHADER_STAGE_FRAGMENT_BIT); })
				});

			multiViewOb.MultiviewPass::createPipeline(
				MultiviewPass::PIPELINE_MULTI,
				vkMethods::TileIndirectCompute0::base::multiViewPipeline,
				renderPass
			);
			return;



			//multisampling = vkMethods::initializers::pipelineMultisampleStateCreateInfo(msaaSamples, 0);
			//pipelineInfo.pMultisampleState = &multisampling;


			multisampling = vkMethods::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
			rasterizer = vkMethods::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
			colorBlendAttachment = vkMethods::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
			depthStencil = vkMethods::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);






			float multiviewArrayLayer = 0.0f;

			VkSpecializationMapEntry specializationMapEntry{ 0, 0, sizeof(float) };

			VkSpecializationInfo specializationInfo{};
			specializationInfo.dataSize = sizeof(float);
			specializationInfo.mapEntryCount = 1;
			specializationInfo.pMapEntries = &specializationMapEntry;
			specializationInfo.pData = &multiviewArrayLayer;

			/*
				Separate pipelines per eye (view) using specialization constants to set view array layer to sample from
			*/
			for (uint32_t i = 0; i < 2; i++) {
				HELPERS_STRING_INS_C({ 's' _CMA_ 'h' _CMA_ 'a' _CMA_ 'd' _CMA_ 'e' _CMA_ 'r' _CMA_ 's' _CMA_ '/' }, 8, { '.' _CMA_ 's' _CMA_ 'p' _CMA_ 'v' }, 4, {
					HELPERS_STRING_INS_F({'v' _CMA_ 'e' _CMA_ 'r' _CMA_ 't' _CMA_ 'G' }, 5, { shaderStages[0] = tools::createShader(_HH_Str, VK_SHADER_STAGE_VERTEX_BIT); })
					HELPERS_STRING_INS_F({'f' _CMA_ 'r' _CMA_ 'a' _CMA_ 'g' _CMA_ 'G' }, 5, { shaderStages[1] = tools::createShader(_HH_Str, VK_SHADER_STAGE_FRAGMENT_BIT); })
					});

				if (!i)
					viewport = vkMethods::initializers::viewport((float)swapChainExtent.width / 2.0f, (float)swapChainExtent.height, 0.0f, 1.0f),
					scissor = vkMethods::initializers::rect2D(swapChainExtent.width / 2, swapChainExtent.height, 0, 0);
				else
					viewport.x = (float)swapChainExtent.width / 2,
					scissor.offset.x = swapChainExtent.width / 2;



				shaderStages[1].pSpecializationInfo = &specializationInfo;
				multiviewArrayLayer = (float)i;
				VkPipelineVertexInputStateCreateInfo emptyInputState = vkMethods::initializers::pipelineVertexInputStateCreateInfo();
				pipelineInfo.pVertexInputState = &emptyInputState;
				pipelineInfo.layout = multiViewOb.pipelineLayout;
				pipelineInfo.renderPass = renderPass;
				vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, vkMethods::TileIndirectCompute0::base::multiViewPipeline + i);

				vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
				vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
			}
		}

		void app::createDescriptorPool() {
			VkDescriptorPoolSize poolSizes[3] = {
			vkMethods::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4),
			vkMethods::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
			};

			VkDescriptorPoolCreateInfo poolInfo = vkMethods::initializers::descriptorPoolCreateInfo(2, poolSizes, 2);

			if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}

		void app::prepareInstanceData() {

			InstanceData instanceData[3];
			//std::vector<InstanceData> instanceData;
			//instanceData.resize(3);
			instanceData[0].pos = glm::vec3(2.0f, 0.0f, 0.0f);
			instanceData[0].rot = glm::vec3(0.0f, 0.0f, 0.0f);
			instanceData[1].pos = glm::vec3(0.0f, 0.0f, 0.0f);
			instanceData[1].rot = glm::vec3(0.0f, 0.0f, 0.0f);
			instanceData[2].pos = glm::vec3(-2.0f, 0.0f, 0.0f);
			instanceData[2].rot = glm::vec3(0.0f, 0.0f, 0.0f);
			instanceBuffer.buffer.create(instanceBuffer.size = 3 * sizeof(InstanceData), instanceData, VkQueueFamShareInfComTransGraph.sData(), transferQueue[0], transferCommandPool[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		void app::createUniformBuffers() {
			for (uint32_t t = 0; t < 2; t++) vkMethods::deviceS::createBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VkQueueFamShareInfComTransGraph.sData(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer[t].buf, uniformBuffer[t].memory);
		}

		void app::createDescriptorSets() {
			VkDescriptorSetLayout layout[2]{ descriptorSetLayout, descriptorSetLayout };

			VkDescriptorSetAllocateInfo allocInfo =
				vkMethods::initializers::descriptorSetAllocateInfo(
					descriptorPool,
					layout,
					2);

			if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}


			VkDescriptorBufferInfo bufferInfo2 = {};
			bufferInfo2.buffer = multiViewOb.uniformBuffer.buf;
			bufferInfo2.offset = 0;
			bufferInfo2.range = sizeof(MultiviewPass::UniformBufferObject);


			VkDescriptorImageInfo imageInfo = vkMethods::initializers::descriptorImageInfo(
				textureS.textureSampler,
				textureS.textureImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			for (uint32_t i = 0; i < 2; i++) {

				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = uniformBuffer[i].buf;
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);
				VkWriteDescriptorSet descriptorWrites[3]{
				vkMethods::initializers::writeDescriptorSet(descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
				vkMethods::initializers::writeDescriptorSet(descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo2),
				vkMethods::initializers::writeDescriptorSet(descriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &imageInfo),
				};
				vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, nullptr);
			}
		}

		void app::updateUniformBuffers(uint32_t i) {

						



			

			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			
			
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			
		
		
			



			

			/*
			if (false &&enableMultiView) {

				vkMethods::MultiviewPass::UniformBufferObject ubo = {};

				ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

				ubo.view[0] = glm::lookAt(glm::vec3(2.02f, 2.0f, 1.98f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

				float aspectRatio = (float)(swapChainExtent.width * 0.5f) / (float)swapChainExtent.height;
				float wd2 = 0.1f * tan(glm::radians(45.0f / 2));
				float ndfl = 0.1f / 0.5f;
				float left, right;
				float top = wd2;
				float bottom = -wd2;
				left = -aspectRatio * wd2 + 0.5f * 0.08f * ndfl;
				right = aspectRatio * wd2 + 0.5f * 0.08f * ndfl;

				ubo.proj[0] = glm::frustum(left, right, bottom, top, 0.1f, 10.0f);
				//ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

				ubo.proj[0][1][1] *= -1;
				left = -aspectRatio * wd2 - 0.5f * 0.08f * ndfl;
				right = aspectRatio * wd2 - 0.5f * 0.08f * ndfl;
				ubo.proj[1] = glm::frustum(left, right, bottom, top, 0.1f, 10.0f);
				ubo.view[1] = glm::lookAt(glm::vec3(1.98f, 2.0f, 2.02f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				ubo.proj[1][1][1] *= -1;

				//ubo.view[0] = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				//ubo.proj[0] = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
				//ubo.view[1] = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				//ubo.proj[1] = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
				ubo.proj[0][1][1] *= -1;
				ubo.proj[1][1][1] *= -1;
				void* data;
				vkMapMemory(device, uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, uniformBuffers[currentImage].memory);
				return;
			}

			*/


			UniformBufferObject ubo = {};





			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));


			void* data;
			vkMapMemory(device, uniformBuffer[i].memory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, uniformBuffer[i].memory);
		}
	}
}