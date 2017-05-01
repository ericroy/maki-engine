#pragma once
#include <cstdlib>
#include <exception>
#include <new>
#include <cassert>
#include <limits>

// #ifdef _CRTDBG_MAP_ALLOC
// #	define _MAKI_ALIGNED_MALLOC(a, b) _aligned_malloc_dbg(a, b, __FILE__, __LINE__)
// #	define _MAKI_ALIGNED_REALLOC(a, b, c) _aligned_realloc_dbg(a, b, c, __FILE__, __LINE__)
// #	define _MAKI_ALIGNED_FREE(a) _aligned_free_dbg(a)
// #else
// #	define _MAKI_ALIGNED_MALLOC _aligned_malloc
// #	define _MAKI_ALIGNED_REALLOC _aligned_realloc
// #	define _MAKI_ALIGNED_FREE _aligned_free
// #endif

namespace maki {
	namespace core {

#if MAKI_OS_WIN

		inline void *aligned_malloc(size_t size, size_t alignment = 8) { return _aligned_malloc(size, alignment); }
		inline void *aligned_realloc(void *existing, size_t size, size_t alignment = 8) { return _aligned_realloc(existing, size, alignment); }
		inline void aligned_free(void *p) { _aligned_free(p); }

#else

		inline void *aligned_malloc(size_t size, size_t alignment = 8) {
			// Alignment must be at least 4, which is the default alignment of malloc for a 32 bit system.
			assert(alignment >= 4);

			// Alignment must be a power of two
			assert((alignment & (alignment - 1)) == 0);

			// Allow for space to store the alignment value before the part of the buffer that we will return.
			char *p = static_cast<char *>(malloc(size + sizeof(size_t) + alignment - 1)) + sizeof(size_t);

			size_t lower_bits = reinterpret_cast<uintptr_t>(p) & (alignment - 1);
			size_t shift = lower_bits != 0 ? alignment - lower_bits : 0;
			p += shift;

			// Stash the alignment value immediately before the 
			*reinterpret_cast<size_t *>(p - sizeof(size_t)) = shift;

			return p;
		}

		inline void *aligned_realloc(void *existing, size_t size, size_t alignment = 8) {
			char *base = nullptr;
			if(existing != nullptr) {
				size_t shift = *reinterpret_cast<size_t *>(static_cast<char *>(existing)-sizeof(size_t));
				base = static_cast<char *>(existing)-shift - sizeof(size_t);
			}

			// Alignment must be at least 4, which is the default alignment of malloc for a 32 bit system.
			assert(alignment >= 4);

			// Alignment must be a power of two
			assert((alignment & (alignment - 1)) == 0);

			// Allow for space to store the alignment value before the part of the buffer that we will return.
			char *p = static_cast<char *>(realloc(base, size + sizeof(size_t) + alignment - 1)) + sizeof(size_t);

			size_t lower_bits = reinterpret_cast<uintptr_t>(p) & (alignment - 1);
			size_t shift = lower_bits != 0 ? alignment - lower_bits : 0;
			p += shift;

			// Stash the alignment value immediately before the 
			*reinterpret_cast<size_t *>(p - sizeof(size_t)) = shift;

			return p;
		}

		inline void aligned_free(void *p) {
			size_t shift = *reinterpret_cast<size_t *>(static_cast<char *>(p) - sizeof(size_t));
			char *base = static_cast<char *>(p) - shift - sizeof(size_t);
			free(base);
		}

#endif

		template<int ALIGNMENT>
		class aligned_t {
		public:
			inline void *operator new(size_t size) {
				void *p = aligned_malloc(size, ALIGNMENT);
				if(p == nullptr)
					throw ::std::bad_alloc();
				return p;
			}
			inline void *operator new(size_t size, const ::std::nothrow_t &) throw() { return aligned_malloc(size, ALIGNMENT); }
			inline void *operator new(size_t size, void *p) throw() { return p; }

			inline void operator delete(void *p) throw() { aligned_free(p); }
			inline void operator delete(void *p, const ::std::nothrow_t&) throw() { aligned_free(p); }
			inline void operator delete(void *p, void *) throw() { aligned_free(p); }

			inline void *operator new[](size_t size) { return aligned_malloc(size, ALIGNMENT); }
			inline void *operator new[](size_t size, const ::std::nothrow_t& nothrow_constant) throw() { return aligned_malloc(size, ALIGNMENT); }
			inline void *operator new[](size_t size, void *p) throw() { return p; }

			inline void operator delete[](void *p) throw() { aligned_free(p); }
			inline void operator delete[](void *p, const ::std::nothrow_t &) throw() { aligned_free(p); }
			inline void operator delete[](void *p, void *) throw() { aligned_free(p); }
		};


		class allocator_t {
		public:
			inline static void *malloc(size_t size, size_t alignment = 8) {
				return aligned_malloc(size, alignment);
			}

			inline static void *realloc(void *p, size_t size, size_t alignment = 8) {
				return aligned_realloc(p, size, alignment = 8);
			}

			inline static void free(void *p) {
				aligned_free(p);
			}
		};


	} // namespace core

} // namespace maki