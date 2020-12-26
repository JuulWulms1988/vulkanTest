#pragma once

#include "Initializers.hpp"
#include "Texture.hpp"
#include "Model.hpp"

#include <string>

namespace vkMethods {
	namespace tools {
		using namespace vkPointers;

		extern std::vector<char> readFile(const std::string& filename);

		inline VkShaderModule createShaderModule(const std::vector<char>& code) {
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			return shaderModule;
		}

		inline VkPipelineShaderStageCreateInfo createShader(std::string fileName, VkShaderStageFlagBits stage) {
			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = createShaderModule(vkMethods::tools::readFile(fileName));
			shaderStage.pName = "main";
			return shaderStage;
		}

		class loadTexTest : public texture {
		public:
			static void load(std::string TEXTURE_PATH, texture& textureS) {
				((loadTexTest*)&textureS)->loadTex(TEXTURE_PATH);
			}

		private:
			void loadTex(std::string TEXTURE_PATH);
		};

		extern inline void loadModelTest(std::string MODEL_PATH, std::vector<model::Vertex>& vertices, std::vector<uint32_t>& indices);
	}
}