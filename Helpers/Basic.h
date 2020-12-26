#pragma once

#include <cstdint>
#include <vector>

#define _CMA_ ,
#define HELPERS_STRING_INS_C(bS, bL, eS, eL, paF) { char _b_b_[] bS, _e_e_[] eS; const uint32_t _bL_bL_ = bL + eL; Helpers::basic::stringInsertCls sTrInCls{ bL, eL}; paF }
#define HELPERS_STRING_INS_F(mS, mL, pF) { char _HH_Str[_bL_bL_ + mL + 1] mS; sTrInCls.out(_HH_Str, mL, _b_b_, _e_e_); pF }
#define HELPERS_STRING(s, pF) { char _H_Str[] = s; pF }

namespace Helpers::basic {
	extern void allocaFunction(void(*ptr)(void*, void*), void* pObj, size_t size);	

	struct stringInsertCls {
		uint32_t beginL;
		uint32_t endL;
		char* out(char* middle, uint32_t l, char* b, char* e) {
			memcpy(middle + beginL, middle, l);
			memcpy(middle, b, beginL);
			memcpy(middle + (l += beginL), e, endL);
			middle[l + endL] = '\0';
			return middle;
		}
	};
}