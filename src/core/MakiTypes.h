#pragma once

namespace Maki
{
	typedef unsigned long long uint64;
	typedef unsigned int uint32;
	typedef unsigned short uint16;
	typedef unsigned char uint8;

	typedef long long int64;
	typedef int int32;
	typedef short int16;
	typedef char int8;


	// Handle to an item in stored in a ResourcePool.
	typedef uint32 Handle;
	const Handle HANDLE_NONE = (uint32)-1;

	// Resource Id
	// A unique identifier for a file resource from the ResourceLibrary
	// Rids cannot be a simple typedef because we need them to have a distinct type from a Handle
	typedef struct _Rid {
		uint32 index;
		inline bool operator==(const _Rid &other) const { return index == other.index; }
		inline bool operator!=(const _Rid &other) const { return index != other.index; }
		inline _Rid &operator=(const uint32 &u) { this->index = u; return *this; }
		inline operator uint32() const { return index; }
	} Rid;
	const Rid RID_NONE = {(uint32)-1};

	const float MAKI_PI = 3.14159265358979323846f;
	const float MAKI_DEG_TO_RAD = MAKI_PI / 180.0f;
	const float MAKI_RAD_TO_DEG = 180.0f / MAKI_PI;

} // namespace Maki
