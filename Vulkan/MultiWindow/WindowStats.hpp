#pragma once

#include "WindowMultiImpl.hpp"
#include <IOStream>

namespace vkMethods {
	class windowStatCls {
	public:
		
		windowStatCls(uint32_t size = 0U) { if ((memSize = size) || (inFlightCount = 0)) std::cout << "jowdieALloc: " << (uint64_t)(point = new uint8_t[seek(TYPE_MAX_ENUM)]) << "\nGrootte: " << seek(TYPE_MAX_ENUM) << "; echte: " << sizeof(windowMultiImpCls) << '\n'; }
		
		uint32_t inFlightCount;
		uint32_t memSize;
		uint8_t* point;

		void resize(uint32_t size) {
			if (!size) {
				if (memSize) delete[seek(TYPE_MAX_ENUM)] point,
					memSize = inFlightCount = 0;
				return;
			}
			windowStatCls newStat(size);
			newStat.inFlightCount = inFlightCount;
			if (memSize) {
				for (uint32_t i = 0; i < TYPE_MAX_ENUM; i++)
					memcpy(
						newStat.point + newStat.seek((types)i),
						point + seek(types(i)),
						memSize < size ? memSize : size * offsetOf((types)i)
					);
				delete[seek(TYPE_MAX_ENUM)] point;
			}
			*this = newStat;
		}

		enum types { TYPE_windowArrayObj, TYPE_pWindowsInFlight, TYPE_presentResult, TYPE_swapChainAr, TYPE_nextImageAr, TYPE_MAX_ENUM };

		template <class pType>
		pType getPoint(types type) { return (pType)(point + seek(type)); }

	private:
		static size_t offsetOf(types type) {
			switch (type) {
			case TYPE_windowArrayObj:		return sizeof(windowMultiImpCls);
			case TYPE_pWindowsInFlight:		return sizeof(windowMultiImpCls*);
			case TYPE_presentResult:		return sizeof(VkResult);
			case TYPE_swapChainAr:			return sizeof(VkSwapchainKHR);
			case TYPE_nextImageAr:			return sizeof(uint32_t) * 2 * swapChainImageCount;			}
		}

		size_t seek(types type) {
			size_t of = 0;
			for (uint32_t i = 0; i < (uint32_t)type; i++) of += offsetOf((types)i);
			return of * memSize;
		}
	} windowStats;
}

//vkMethods::windowMultiImpCls* windowArrayObj;

	//std::vector<vkMethods::windowMultiImpCls> windowArrayObj;

	//std::vector<vkMethods::windowMultiImpCls*> pWindowsInFlight;
	//vkresult
	//swapchain
	//std::array<std::vector<std::vector<uint32_t>>, 2> nextImageAr;