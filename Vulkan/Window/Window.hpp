#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <IOStream>

namespace vkMethods {
	namespace vkPointers {
		extern VkInstance vulkanInstance;
	}

	class windowCls {
	public:
		static void initGlfw() {
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		
		virtual void init() {
			instWin = glfwCreateWindow(instWidth, instHeight, "Vulkan", nullptr, nullptr);
			glfwSetWindowUserPointer((GLFWwindow*)instWin, this);
			glfwSetFramebufferSizeCallback((GLFWwindow*)instWin, framebufferResizeCallback);
			
			glfwCreateWindowSurface(vkPointers::vulkanInstance, (GLFWwindow*)instWin, nullptr, &instSurface);
		}

	protected:
		static const int instWidth = 1600;
		static const int instHeight = 600;
		VkSurfaceKHR instSurface;

	private:
		void* instWin;
		bool framebufferResized = false;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<windowCls*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}
	};

}