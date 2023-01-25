#define VMA_IMPLEMENTATION 1

//#define USE_VMA_DEBUG
#ifdef USE_VMA_DEBUG

	#define VMA_DEBUG_LOG(format, ...) do {\
		printf(format, __VA_ARGS__);\
		printf("\n"); \
	} while(false);

#endif


#include "vk_mem_alloc.h"