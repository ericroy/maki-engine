#pragma once
#include "core/MakiAllocator.h"

#define MAKI_SAFE_DELETE(x) if((x)) { delete (x); (x) = nullptr; }
#define MAKI_SAFE_FREE(x) if((x)) { Core::allocator_t::free((x)); (x) = nullptr; }
#define MAKI_SAFE_DELETE_ARRAY(x) if((x)) { delete[] (x); (x) = nullptr; }

#define MAKI_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#define MAKI_TO_RID(x) *(rid_t *)&(x)
