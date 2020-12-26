#pragma once

#include "TileIndirectCompute0Base.h"
#include <chrono>
#define CONSTR_TILEINDIRECTCOMPUTE0 *[]{ static app x; return &x; }()

namespace vkMethods {
	namespace TileIndirectCompute0 {
		
		class app : public base {
		public:
			struct InstanceData {
				glm::vec3 pos;
				glm::vec3 rot;
			};
			static void initVulkan();
			static void reCreate(void(*engineRecreate)(void*), void* pObj);
			static void cleanup();
			static void render();

		private:
			void init();
			void reCreateObj();
			void cleanupObj();
			void renderObj();
			void prepareInstanceData();
			void createUniformBuffers();
			void createDescriptorSets();
			void updateUniformBuffers(uint32_t i);
			
			static void createDescriptorSetLayout();
			static void createGraphicsPipeline();
			static void createDescriptorPool();
			

			static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
		};
	}
}