#pragma once
#include "core/MakiAllocator.h"

#define SAFE_DELETE(x) if((x)) { delete (x); (x) = nullptr; }
#define SAFE_FREE(x) if((x)) { Core::Allocator::Free((x)); (x) = nullptr; }
#define SAFE_DELETE_ARRAY(x) if((x)) { delete[] (x); (x) = nullptr; }

#define SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#define TO_RID(x) *(Rid *)&(x)
