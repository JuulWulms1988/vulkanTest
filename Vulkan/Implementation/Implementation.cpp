#include "Implementation.h"

const bool enableMultiView = true;
//const bool enableMultiView2 = true;

namespace vkMethods::vkPointers {
	MultiviewPass& multiViewOb = *new MultiviewPass;
	syncUpdateCompute::base& syncUpdateComputeOb = *new syncUpdateCompute::app;

	extern size_t currentFrame;
}

using namespace vkMethods::vkPointers;

int mainEngine() {
	vulkanBase& app = *[] { static vulkanApp X; return &X; }();

	try {
		app.run();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void vulkanApp::render() {
	vulkanBase::prepareDraw([](uint32_t i) { return vkMethods::windowStats.getPoint<vkMethods::windowMultiImpCls**>
		(
			vkMethods::windowStatCls::TYPE_pWindowsInFlight
			)[i]->acquireNext((uint32_t)currentFrame); }, vkMethods::windowStats.inFlightCount);
	imageIndex = vkMethods::windowStats.getPoint<vkMethods::windowMultiImpCls**>
		(
			vkMethods::windowStatCls::TYPE_pWindowsInFlight
			)[0]->imageIndex;
	submitInfo.waitSemaphoreCount = submitInfo.signalSemaphoreCount = 0;
	drawFrame();
	vulkanBase::finishDraw();
}



void vulkanApp::drawFrame() {
	//std::cout << "imind: " << imageIndex << "\ncurFrame: " << currentFrame << "\nres: " << imageIndex + swapChainImageCount * currentFrame << "\n\n";
	vkMethods::TileIndirectCompute0::app::render();
	VkPipelineStageFlags f = VK_PIPELINE_STAGE_TRANSFER_BIT;
	
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex + swapChainImageCount * currentFrame];
	submitInfo.commandBufferCount = 1;
	vkQueueSubmit(currentFrame ? graphicsQueue : presentQueue, 1, &submitInfo, VK_NULL_HANDLE);
}

void vulkanApp::initVulkan() {
	{
		VkCommandPoolCreateFlags tr[1]{ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT }, co[1]{ 0 }, gr[1]{ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
		vkMethods::info::VkBaseCreateInfo baseInfo = {};
		baseInfo.VkQueueCount_Trans = 2;
		baseInfo.VkComPoolCount_Trans = 1;
		baseInfo.VkComPoolCount_Graph = 1;
		baseInfo.VkComPoolCount_Compute = 1;
		baseInfo.transQueuesFlags = tr;
		baseInfo.VkQueueCount_Graph = 2;
		baseInfo.VkQueueCount_Compute = 4;
		baseInfo.computeQueuesFlags = co;
		baseInfo.graphQueuesFlags = gr;
		vulkanBase::initVulkanBase(&baseInfo);
	}
	createDescriptorPool();
	
	syncUpdateComputeOb.init();
	if (enableMultiView) { 
		VkFormat df; 
		vkMethods::deviceS::getSupportedDepthFormat(physicalDevice, &df);
		multiViewOb.prepare(df);
		multiViewOb.createDescriptor();
	}

	vkMethods::TileIndirectCompute0::app::initVulkan();

	vkMethods::windowStats.resize(1);

	vkMethods::windowMultiImpCls* pointWin0 = vkMethods::windowStats.getPoint<vkMethods::windowMultiImpCls*>
		(
			vkMethods::windowStatCls::TYPE_windowArrayObj
			);
	vkMethods::windowMultiImpCls ii = vkMethods::windowMultiImpCls();
	pointWin0[0] = ii;
	typedef uint32_t kaas[2][3];
	
	kaas* nextImageAr = vkMethods::windowStats.getPoint<kaas*>
		(
			vkMethods::windowStatCls::TYPE_nextImageAr
			);
	for
		(
			uint32_t I,
			i = I = 0;
			i < swapChainImageCount || (!(i = 0) && ++I < 2);
			i++
			) nextImageAr[I][i][0] = i;
	
	vkMethods::windowStats.inFlightCount = 1;


	vkMethods::windowStats.getPoint<vkMethods::windowMultiImpCls**>
		(
			vkMethods::windowStatCls::TYPE_pWindowsInFlight
			)[0] = pointWin0;
	
	//alvast 2
	
	
	if (enableMultiView) multiViewOb.createCommandBuffers(), multiViewOb.buildCommandBuffers(vkMethods::TileIndirectCompute0::base::multiViewPipeline);
	
	pointWin0[0].blitPassCls::viewT = vkMethods::multiView::blitPassCls::VIEWTYPE_MULTI;
	std::cout << "EindPointWin0: " << (uint64_t)pointWin0 << "\nen de point:" << (uint64_t)vkMethods::windowStats.point << '\n';
	//pointWin0[0].init();

	vkMethods::windowMultiImpCls* aAaa = (vkMethods::windowMultiImpCls*) new unsigned char [sizeof(vkMethods::windowMultiImpCls)];
	*aAaa = ii;
	new vkMethods::windowMultiImpCls(*aAaa);
	aAaa->init();

	while (true);
	buildCommandBuffers();
}

void vulkanApp::cleanup() {
	vkMethods::TileIndirectCompute0::app::cleanup();
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vulkanBase::cleanup();
}

void vulkanApp::reCreate() {
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkMethods::TileIndirectCompute0::app::reCreate([](void* pObj) {	((vulkanBase*)pObj)->vulkanBase::reCreate(); }, this);


	buildCommandBuffers();
}



void vulkanApp::buildCommandBuffers() {
	struct inf {
		vulkanApp* pThis;
		uint32_t i;
	} info{ this };
	for (size_t i = 0, m = commandBuffers.size(); i < m; i++)
		info.i = (uint32_t)i, Helpers::basic::allocaFunction([](void* ptrInf, void* ptr) {
		vulkanApp* ptrThis = ((inf*)ptrInf)->pThis;
		uint32_t i = ((inf*)ptrInf)->i;

		VkCommandBufferBeginInfo beginInfo = vkMethods::initializers::commandBufferBeginInfo();
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		vkBeginCommandBuffer(ptrThis->commandBuffers[i], &beginInfo);
		
		vkMethods::windowMultiImpCls::uploadCommands({
			vkMethods::windowStats.getPoint<vkMethods::windowMultiImpCls**>
			(
				vkMethods::windowStatCls::TYPE_pWindowsInFlight
			),
			vkMethods::windowStats.inFlightCount,
			vkMethods::windowStats.getPoint<uint32_t***>
			(
				vkMethods::windowStatCls::TYPE_nextImageAr
			)[i >= swapChainImageCount][i % swapChainImageCount],
			(VkImageMemoryBarrier*)ptr,
			ptrThis->commandBuffers[i],
			i >= swapChainImageCount }
		), ptrThis->uploadCommands(i);

		vkEndCommandBuffer(ptrThis->commandBuffers[i]);

		}, &info, sizeof(VkImageMemoryBarrier) * vkMethods::windowStats.inFlightCount);
}

void vulkanApp::uploadCommands(uint32_t i) {


	
	std::cout << "yoyo2\n";
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;


	renderPassInfo.renderPass = multiViewOb.renderPass;
	renderPassInfo.framebuffer = multiViewOb.frameBuffer[i >= swapChainImageCount];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	vkCmdExecuteCommands(commandBuffers[i], 1, &vkMethods::TileIndirectCompute0::base::pTile_Main.commandBuffers[i]);

	vkCmdEndRenderPass(commandBuffers[i]);

	
}

void vulkanApp::createDescriptorPool() {
	VkDescriptorPoolSize poolSizes[3] = {
	vkMethods::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6),
	};

	VkDescriptorPoolCreateInfo poolInfo = vkMethods::initializers::descriptorPoolCreateInfo(1, poolSizes, 3);

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &mainDescriptorPool);
}