#pragma once
#include <stdint.h>

namespace maki
{
	// handle_t to an item in stored in a resource_pool_t.
	typedef uint64_t handle_t;
	const handle_t HANDLE_NONE = (uint64_t)-1;

	// resource_t Id
	// A unique identifier for a file resource from the resource_library_t
	// Rids cannot be a simple typedef because we need them to have a distinct type from a handle_t
	typedef struct _Rid {
		uint64_t index_;
		inline bool operator==(const _Rid &other) const { return index_ == other.index_; }
		inline bool operator!=(const _Rid &other) const { return index_ != other.index_; }
		inline _Rid &operator=(const uint64_t &u) { index_ = u; return *this; }
		inline operator uint64_t() const { return index_; }
	} rid_t;
	const rid_t RID_NONE = {(uint64_t)-1};

	const float MAKI_PI = 3.14159265358979323846f;
	const float MAKI_DEG_TO_RAD = MAKI_PI / 180.0f;
	const float MAKI_RAD_TO_DEG = 180.0f / MAKI_PI;

	// For 2d things like sprites and flash movies, this is the scale
	// factor used to size the 2d art in a 3d world.  PPU stands for
	// 'pixels per unit'.
	const float MAKI_PPU = 150.0f;
	
} // namespace maki
