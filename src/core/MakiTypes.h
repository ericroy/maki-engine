#pragma once
#include <cassert>
#include <stdint.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <functional>

using std::vector;
using std::string;
using std::unique_ptr;
using std::move;
using std::forward;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::function;


#define MAKI_SAFE_DELETE(x) if((x)) { delete (x); (x) = nullptr; }

#define MAKI_SAFE_FREE(x) if((x)) { core::allocator_t::free((x)); (x) = nullptr; }

#define MAKI_SAFE_DELETE_ARRAY(x) if((x)) { delete[] (x); (x) = nullptr; }

#define MAKI_SAFE_RELEASE(x) if((x)) { (x)->Release(); (x) = nullptr; }

#define MAKI_TO_RID(x) *(rid_t *)&(x)

#define MAKI_NO_COPY(TYPENAME) \
	TYPENAME(const TYPENAME &) = delete;\
	void operator=(const TYPENAME &) = delete;

#define MAKI_ASSERT(X) assert(X)


namespace maki {

	typedef uint32_t handle_t;
	const handle_t HANDLE_NONE = (handle_t)-1;

	typedef uint16_t ref_count_t;
	const ref_count_t REF_COUNT_MAX = ::std::numeric_limits<ref_count_t>::max();

	// Resource id
	// A unique identifier for a file resource from the resource_library_t
	// Rids cannot be a simple typedef because we need them to have a distinct type from a handle_t
	typedef struct _rid_t {
		uint32_t index_;
		inline bool operator==(const _rid_t &other) const { return index_ == other.index_; }
		inline bool operator!=(const _rid_t &other) const { return index_ != other.index_; }
		inline _rid_t &operator=(const uint32_t &u) { index_ = u; return *this; }
		inline operator uint32_t() const { return index_; }
	} rid_t;
	const rid_t RID_NONE = {(uint32_t)-1};

	const float MAKI_PI = 3.14159265358979323846f;
	const float MAKI_DEG_TO_RAD = MAKI_PI / 180.0f;
	const float MAKI_RAD_TO_DEG = 180.0f / MAKI_PI;

	// For 2d things like sprites and flash movies, this is the scale
	// factor used to size the 2d art in a 3d world.  PPU stands for
	// 'pixels per unit'.
	const float MAKI_PPU = 150.0f;
	
} // namespace maki
