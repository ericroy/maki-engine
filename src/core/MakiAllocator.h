#pragma once
#include "core/core_stdafx.h"
#include <cstdlib>
#include <exception>
#include <new>
#include <cassert>
#include <limits>

#define SIMD_ALIGN 16
#define CACHE_ALIGN 64

// #ifdef _CRTDBG_MAP_ALLOC
// #	define _MAKI_ALIGNED_MALLOC(a, b) _aligned_malloc_dbg(a, b, __FILE__, __LINE__)
// #	define _MAKI_ALIGNED_REALLOC(a, b, c) _aligned_realloc_dbg(a, b, c, __FILE__, __LINE__)
// #	define _MAKI_ALIGNED_FREE(a) _aligned_free_dbg(a)
// #else
// #	define _MAKI_ALIGNED_MALLOC _aligned_malloc
// #	define _MAKI_ALIGNED_REALLOC _aligned_realloc
// #	define _MAKI_ALIGNED_FREE _aligned_free
// #endif

namespace Maki
{
	namespace Core
	{

		inline void *AlignedMalloc(std::size_t size, std::size_t alignment = 8)
		{
			// Alignment must be at least 4, which is the default alignment of malloc for a 32 bit system.
			assert(alignment >= 4);

			// Alignment must be a power of two
			assert((alignment & (alignment-1)) == 0);

			// Allow for space to store the alignment value before the part of the buffer that we will return.
			char *p = static_cast<char *>(malloc(size + sizeof(std::size_t) + alignment-1)) + sizeof(std::size_t);

			std::size_t lowerBits = reinterpret_cast<uintptr_t>(p) & (alignment-1);
			std::size_t shift = lowerBits != 0 ? alignment - lowerBits : 0;
			p += shift;

			// Stash the alignment value immediately before the 
			*reinterpret_cast<std::size_t *>(p - sizeof(std::size_t)) = shift;

			return p;
		}

		inline void *AlignedRealloc(void *existing, std::size_t size, std::size_t alignment = 8)
		{
			char *base = nullptr;
			if(existing != nullptr) {
				std::size_t shift = *reinterpret_cast<std::size_t *>(static_cast<char *>(existing)-sizeof(std::size_t));
				base = static_cast<char *>(existing)-shift - sizeof(std::size_t);
			}

			// Alignment must be at least 4, which is the default alignment of malloc for a 32 bit system.
			assert(alignment >= 4);

			// Alignment must be a power of two
			assert((alignment & (alignment-1)) == 0);

			// Allow for space to store the alignment value before the part of the buffer that we will return.
			char *p = static_cast<char *>(realloc(base, size + sizeof(std::size_t) + alignment-1)) + sizeof(std::size_t);

			std::size_t lowerBits = reinterpret_cast<uintptr_t>(p) & (alignment-1);
			std::size_t shift = lowerBits != 0 ? alignment - lowerBits : 0;
			p += shift;

			// Stash the alignment value immediately before the 
			*reinterpret_cast<std::size_t *>(p - sizeof(std::size_t)) = shift;

			return p;
		}

		inline void AlignedFree(void *p)
		{
			std::size_t shift = *reinterpret_cast<std::size_t *>(static_cast<char *>(p) - sizeof(std::size_t));
			char *base = static_cast<char *>(p) - shift - sizeof(std::size_t);
			free(base);
		}




		template<int Align>
		class Aligned
		{
		public:
			inline void *operator new(std::size_t size)
			{
				void *p = AlignedMalloc(size, Align);
				if(p == nullptr)
				{
					throw std::bad_alloc();
				}
				return p;
			}
			inline void *operator new(std::size_t size, const std::nothrow_t &) throw() { return AlignedMalloc(size, Align); }
			inline void *operator new(std::size_t size, void *p) throw() { return p; }

			inline void operator delete(void *p) throw() { AlignedFree(p); }
			inline void operator delete(void *p, const std::nothrow_t&) throw() { AlignedFree(p); }
			inline void operator delete(void *p, void *) throw() { AlignedFree(p); }

			inline void *operator new[](std::size_t size) { return AlignedMalloc(size, Align); }
			inline void *operator new[](std::size_t size, const std::nothrow_t& nothrow_constant) throw() { return AlignedMalloc(size, Align); }
			inline void *operator new[](std::size_t size, void *p) throw() { return p; }

			inline void operator delete[](void *p) throw() { AlignedFree(p); }
			inline void operator delete[](void *p, const std::nothrow_t &) throw() { AlignedFree(p); }
			inline void operator delete[](void *p, void *) throw() { AlignedFree(p); }
		};

		class Allocator
		{
		public:
			inline static void *Malloc(std::size_t size, std::size_t alignment = 8)
			{
				return AlignedMalloc(size, alignment);
			}

			inline static void *Realloc(void *p, std::size_t size, std::size_t alignment = 8)
			{
				return AlignedRealloc(p, size, alignment = 8);
			}

			inline static void Free(void *p)
			{
				AlignedFree(p);
			}
		};


	} // namespace Core

} // namespace Maki