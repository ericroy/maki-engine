#pragma once
#include <cstdio>
#include "core/MakiAllocator.h"

#define MAKI_SAFE_DELETE(x) if((x)) { delete (x); (x) = nullptr; }
#define MAKI_SAFE_FREE(x) if((x)) { core::allocator_t::free((x)); (x) = nullptr; }
#define MAKI_SAFE_DELETE_ARRAY(x) if((x)) { delete[] (x); (x) = nullptr; }

#define MAKI_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#define MAKI_TO_RID(x) *(rid_t *)&(x)

#define MAKI_NO_COPY(TYPENAME) \
	TYPENAME(const TYPENAME &) = delete;\
	void operator=(const TYPENAME &) = delete;

#define MAKI_ASSERT(X) assert(X)