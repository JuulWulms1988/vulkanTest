#pragma once

#include "vulkan/vulkan.h"
#include "Buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace vkMethods {
	//namespace vkPointers {
	//	extern VkPhysicalDevice physicalDevice;
	//	extern VkDevice device;
	//}
		using namespace vkPointers;

		class model {
		public:
			vkMethods::buffer vertices;
			vkMethods::buffer indices;
			struct Vertex {
				glm::vec3 pos;
				glm::vec3 color;
				glm::vec2 texCoord;

				bool operator==(const Vertex& other) const {
					return pos == other.pos && color == other.color && texCoord == other.texCoord;
				}
			};
		};
}
