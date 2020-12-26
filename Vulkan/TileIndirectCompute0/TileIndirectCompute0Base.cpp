#include "TileIndirectCompute0Base.h"



namespace vkMethods::TileIndirectCompute0 {
	VkDescriptorPool base::descriptorPool;
	VkPipeline base::multiViewPipeline[2];
	VkPipeline base::graphicsPipeline;
	VkDescriptorSetLayout base::descriptorSetLayout;

	void base::init() {
		createCommandBuffers();
		buildCommandBuffers();
	}



	inline void base::createCommandBuffers() {
		commandBuffers.resize(swapChainImageCount * 2);
		if (vkAllocateCommandBuffers(
			device, 
			&(vkMethods::initializers::commandBufferAllocateInfo(
				commandPool, 
				VK_COMMAND_BUFFER_LEVEL_SECONDARY, swapChainImageCount * 2)), 
			commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void base::reCreateObj() {
		vkFreeCommandBuffers(device, commandPool, swapChainImageCount * 2, commandBuffers.data());
		createCommandBuffers();
		buildCommandBuffers();
	}

	void base::reCreate() {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	void base::cleanup() {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		pTile_Main.cleanupObj();
		
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

	void base::cleanupObj() {
		vkFreeCommandBuffers(device, commandPool, swapChainImageCount * 2, commandBuffers.data());
		vkDestroySampler(device, textureS.textureSampler, nullptr);
		vkDestroyImageView(device, textureS.textureImageView, nullptr);

		vkDestroyImage(device, textureS.textureImage, nullptr);
		vkFreeMemory(device, textureS.textureImageMemory, nullptr);
		instanceBuffer.buffer.destroy();

		modelS.indices.destroy();

		modelS.vertices.destroy();
		for (uint32_t t = 0; t < 2; t++) uniformBuffer[t].destroy();
	}

	void base::render() {
		pTile_Main.renderObj();
	}

	void base::renderObj() {
	
	}

	void base::buildCommandBuffers() {

		for (uint32_t i = 0; i < (uint32_t)swapChainImageCount * 2; i++) {
			VkCommandBufferInheritanceInfo inInfo = vkMethods::initializers::commandBufferInheritanceInfo();


			VkCommandBufferBeginInfo beginInfo = vkMethods::initializers::commandBufferBeginInfo();
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			inInfo.renderPass = multiViewOb.renderPass;
			inInfo.framebuffer = multiViewOb.frameBuffer[i >= swapChainImageCount];
			beginInfo.pInheritanceInfo = &inInfo;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i >= swapChainImageCount], 0, nullptr);

			VkBuffer vertexBuffers[]{ modelS.vertices.buf };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, &instanceBuffer.buffer.buf, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], modelS.indices.buf, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 3, 0, 0, 0);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to recordSpec command buffer!");
			}
		}
	}

	

}