#include "RenderBufferSwap0Base.h"

namespace vkMethods::renderBufferSwap0 {


	void VkRenderBufSwap0Base::initBase(uint32_t cnt) {

		updateNext = (VkCommandBuffer(*)[2][3]) malloc((dataCount = cnt) * ((sizeof(VkCommandBuffer) * 15 + sizeof(updateStatsStr))));
		updateData = (VkCommandBuffer(*)[3][3])(updateNext + 2);
		updateStats = (updateStatsStr*)(updateData + 3);

		createCB();
	}

	VkRenderBufSwap0Base::~VkRenderBufSwap0Base() {
		delete[dataCount * (sizeof(VkCommandBuffer) * 15 + sizeof(updateStatsStr))] updateNext;
	}

	uint32_t VkRenderBufSwap0Base::comBufCntExec(const VkCommandBuffer*& pComBuf, uint32_t imgInd) {
		pUpS in = *pUp;
		return pUpReady[imgInd] = pComBuf = in.up[imgInd], in.cnt;
	}

	void VkRenderBufSwap0Base::update(uint32_t statIndex, uint32_t dataIndex, void(*pFunc)(void*, VkCBUpSet), void* pObj) {
		shared_mut.lock_shared();
		VkCBUpSet v;
		{
			uint32_t c = updateStats[statIndex].offSet + dataIndex;
			for (uint32_t t = 0; t < 3; t++) v[t] = updateNext[1][t][updateStats[statIndex].offSet + dataIndex];
			bool shed;
			{
				shared_mut.unlock_shared();
				pFunc(pObj, v);
				std::unique_lock<std::mutex> lk(mut);
				conditionPBufferUpdate.wait(lk, [&] { return updateNext[0][0][c] != updateNext[1][0][c] || !(triggerUpdateFlag = true); });
				shared_mut.lock();
				for (uint32_t t = 0; t < 3; t++)
					updateData[(pUp->up - *updateData) % 3][t][updateStats[statIndex].offSet + dataIndex] = v[t],
					updateNext[1][t][c] = updateNext[0][t][c];
				shared_mut.unlock();
				shed = runUpFlag == RBUF_RUN_UP_IDDLE;
				runUpFlag = RBUF_RUN_UP_UPDATE;
			}
			if (shed) shedule();
		}
	}

	void VkRenderBufSwap0Base::renderBufferFunc() {
		std::unique_lock<std::mutex> lk(mut);
		do {
			if (runUpFlag == RBUF_RUN_UP_UPDATE) {
				runUpFlag = RBUF_RUN_UP_RUNNING;

			}

		} while (runUpFlag);
	}

	void VkRenderBufSwap0Base::createCB() {
		for (uint32_t t = 0; t < 3; t++)
			if (vkAllocateCommandBuffers(device, &(vkMethods::initializers::commandBufferAllocateInfo(_VK_RENDER_BUFFERSWAP0_COMPOOL(t), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 3)), bufCB + t * 3) != VK_SUCCESS)
				HELPERS_STRING({ 'f' _CMA_ 'a' _CMA_ 'i' _CMA_ 'l' _CMA_ 'e' _CMA_ 'd' _CMA_ ' ' _CMA_ 't' _CMA_ 'o' _CMA_ ' '
					_CMA_ 'a' _CMA_ 'l' _CMA_ 'l' _CMA_ 'o' _CMA_ 'c' _CMA_ 'a' _CMA_ 't' _CMA_ 'e' _CMA_ ' ' _CMA_ 'c' _CMA_ 'o'
					_CMA_ 'm' _CMA_ 'm' _CMA_ 'a' _CMA_ 'n' _CMA_ 'd' _CMA_ ' ' _CMA_ 'b' _CMA_ 'u' _CMA_ 'f' _CMA_ 'f' _CMA_ 'e'
					_CMA_ 'r' _CMA_ 's' _CMA_ '!' }, { throw std::runtime_error(_H_Str); })
				// failed to allocate command buffers!
	}
}