#pragma once

#include "vulkan/vulkan.h"
#include "..\\Base\Tools.h"
#include "..\\..\\Helpers\\Basic.h"
#include "..\\Multiview\MultiView.hpp"

#include <vector>

extern const bool enableMultiView;

namespace vkMethods {
	namespace vkPointers {
		extern MultiviewPass& multiViewOb;
		extern std::vector<VkFramebuffer> swapChainFramebuffers;
		extern VkRenderPass renderPass;
		extern VkSampleCountFlagBits msaaSamples;
	}
	namespace TileIndirectCompute0 {
		using namespace vkPointers;

		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
		};
		
		class base {
		public:
			std::vector<VkCommandBuffer> commandBuffers;
				
			virtual void init();
			virtual void reCreateObj();
			virtual void cleanupObj();
			
			static base& pTile_Main;
			static VkPipeline graphicsPipeline;

			static VkPipeline multiViewPipeline[2];
			
		protected:
			
			vkMethods::texture textureS;
			std::vector<vkMethods::model::Vertex> vertices;
			

			vkMethods::buffer uniformBuffer[2];
			
			vkMethods::model modelS;
			struct InstanceBuffer { vkMethods::buffer buffer; size_t size = 0; VkDescriptorBufferInfo descriptor; } instanceBuffer;
			std::vector<uint32_t> indices;
			
			static VkDescriptorPool descriptorPool;
			static VkDescriptorSetLayout descriptorSetLayout;
	
			static void reCreate();
			static void cleanup();
			static void render();
			//static VkPipelineShaderStageCreateInfo createShaderModule(std::string fileName, VkShaderStageFlagBits stage);
		private:
			
			void createCommandBuffers();
			void buildCommandBuffers();
			virtual void renderObj();
		};
	}
 }