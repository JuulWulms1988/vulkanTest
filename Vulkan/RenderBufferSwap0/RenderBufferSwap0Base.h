#pragma once

#include "vulkan/vulkan.h"
#include "..\\Base\Initializers.hpp"
#include "..\\..\\Helpers\Basic.h"
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

#ifndef _VK_RENDER_BUFFERSWAP0_COMPOOL
#define _VK_RENDER_BUFFERSWAP0_COMPOOL(c) VK_NULL_HANDLE
#endif

namespace vkMethods::vkPointers {
	extern VkDevice device;
}

namespace vkMethods::renderBufferSwap0 {
	using namespace vkPointers;

	class VkRenderBufSwap0Base {
	public:
		struct updateStatsStr {
			uint32_t offSet;
			uint32_t EndoffSet;
		};

		typedef VkCommandBuffer VkCBUpSet[3];

		void initBase(uint32_t cnt); 

		~VkRenderBufSwap0Base();
		
		uint32_t comBufCntExec(const VkCommandBuffer*& pComBuf, uint32_t imgInd);

		

	protected:
		std::shared_mutex shared_mut;
		std::mutex mut;

		virtual void shedule() = 0;

		void update(uint32_t statIndex, uint32_t dataIndex, void(*pFunc)(void*, VkCBUpSet), void* pObj);

		void renderBufferFunc();

	private:
		VkCommandBuffer (*updateNext)[2][3];
		VkCommandBuffer (*updateData) [3][3] = nullptr;
		updateStatsStr* updateStats;
		uint32_t updateCount = 0;

		uint32_t dataCount;

		enum { RBUF_RUN_UP_IDDLE, RBUF_RUN_UP_RUNNING, RBUF_RUN_UP_UPDATE };
		uint8_t runUpFlag = RBUF_RUN_UP_IDDLE;

		typedef struct { const VkCommandBuffer(*up)[3]; uint32_t cnt; } pUpS;
				
		pUpS* volatile pUp;
		const VkCommandBuffer* volatile pUpReady[3];

		bool triggerUpdateFlag = 0;
		volatile bool triggerFlag = 0;
		
		enum waitStageFlags { RBUF_WSTAGE_NONE, RBUF_WSTAGE_BUF0_BIT, RBUF_WSTAGE_BUF1_BIT = 3, RBUF_WSTAGE_BUF2_BIT = 7 };
		uint8_t waitStateCnt = 0;

		std::condition_variable conditionPBuffer;
		std::condition_variable conditionPBufferUpdate;
		VkCommandBuffer bufCB[9];

		void createCB();

	};
}