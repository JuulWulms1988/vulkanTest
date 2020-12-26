#pragma once

#include "..\\Base\\Base.h"
#include "..\\Base\\Texture.hpp"
#include "..\\Base\\Model.hpp"
#include "..\\TileIndirectCompute0\\TileIndirectCompute0Implementation.h"
#include "..\\MultiWindow\WindowStats.hpp"
#include "..\\SyncUpdateCompute\SyncUpdateComputeImpl.hpp"

#include <mutex>

using namespace vkMethods::vkPointers;




class vulkanApp : public vulkanBase {
public:
	void initVulkan();
	void render();

private:
	void buildCommandBuffers();
	void uploadCommands(uint32_t i);
	void drawFrame();
	void cleanup();
	void reCreate();
	void createDescriptorPool();
};