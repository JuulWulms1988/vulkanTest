#pragma once

#include "Window.hpp"
#include "..\\Base\Swapchain.hpp"

namespace vkMethods {
	class windowSwapChainCls : windowCls {
	public:


		VkExtent2D swapChainExtent;
		std::vector<VkImage> swapChainImages;


		void init() {
			windowCls::init();
			swapchain::createSwapChain(
				swapChain, 
				swapChainImages, 
				windowCls::instSurface, 
				swapChainExtent, 
				swapChainImageFormat, 
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			);

			swapchain::createImageViews(
				swapChainImageViews, 
				swapChainImages, 
				swapChainImageFormat
			);
		}

	protected:
		VkSwapchainKHR swapChain;
	
		std::vector<VkImageView> swapChainImageViews;
		
	private:
		VkFormat swapChainImageFormat;
	};
}