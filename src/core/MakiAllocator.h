#pragma once
#include "core/core_stdafx.h"
#include <cstdlib>
#include <exception>
#include <new>

#define SIMD_ALIGN 16
#define CACHE_ALIGN 64

#ifdef _CRTDBG_MAP_ALLOC
#	define _MAKI_ALIGNED_MALLOC(a, b) _aligned_malloc_dbg(a, b, __FILE__, __LINE__)
#	define _MAKI_ALIGNED_REALLOC(a, b, c) _aligned_realloc_dbg(a, b, c, __FILE__, __LINE__)
#	define _MAKI_ALIGNED_FREE(a) _aligned_free_dbg(a)
#else
#	define _MAKI_ALIGNED_MALLOC _aligned_malloc
#	define _MAKI_ALIGNED_REALLOC _aligned_realloc
#	define _MAKI_ALIGNED_FREE _aligned_free
#endif

namespace Maki
{
	namespace Core
	{

		template<int Align>
		class Aligned
		{
		public:
			inline void *operator new(std::size_t size)
			{
				void *p = _aligned_malloc(size, Align);
				if(p == nullptr)
				{
					throw std::bad_alloc();
				}
				return p;
			}
			inline void *operator new(std::size_t size, const std::nothrow_t &) throw() { return _MAKI_ALIGNED_MALLOC(size, Align); }
			inline void *operator new(std::size_t size, void *p) throw() { return p; }

			inline void operator delete(void *p) throw() { _MAKI_ALIGNED_FREE(p); }
			inline void operator delete(void *p, const std::nothrow_t&) throw() { _MAKI_ALIGNED_FREE(p); }
			inline void operator delete(void *p, void *) throw() { _MAKI_ALIGNED_FREE(p); }

			inline void *operator new[](std::size_t size) { return _MAKI_ALIGNED_MALLOC(size, Align); }
			inline void *operator new[](std::size_t size, const std::nothrow_t& nothrow_constant) throw() { return _MAKI_ALIGNED_MALLOC(size, Align); }
			inline void *operator new[](std::size_t size, void *p) throw() { return p; }

			inline void operator delete[](void *p) throw() { _MAKI_ALIGNED_FREE(p); }
			inline void operator delete[](void *p, const std::nothrow_t &) throw() { _MAKI_ALIGNED_FREE(p); }
			inline void operator delete[](void *p, void *) throw() { _MAKI_ALIGNED_FREE(p); }
		};

		class Allocator
		{
		public:
			inline static void *Malloc(std::size_t size, std::size_t alignment = 8)
			{
				return _MAKI_ALIGNED_MALLOC(size, alignment);
			}

			inline static void *Realloc(void *p, std::size_t size, std::size_t alignment = 8)
			{
				return _MAKI_ALIGNED_REALLOC(p, size, alignment = 8);
			}

			inline static void Free(void *p)
			{
				_MAKI_ALIGNED_FREE(p);
			}
		};


	} // namespace Core

} // namespace Maki