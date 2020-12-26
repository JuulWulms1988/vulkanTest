#include "Basic.h"

void Helpers::basic::allocaFunction(void(*ptr)(void*, void*), void* pObj, size_t size) {
	ptr(pObj, alloca(size));
}