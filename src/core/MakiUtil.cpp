#include "core/core_stdafx.h"
#include "core/MakiUtil.h"

#if defined(_WIN32) || defined(_WIN64)
#	define NOMINMAX 1
#	include <Windows.h>
#endif

namespace Maki
{
	namespace Core
	{

		void *MakiLoadLibrary(const char *libName)
		{
			void *ret = nullptr;
		#if defined(_WIN32) || defined(_WIN64)
			ret = LoadLibraryA(libName);
		#else

		#endif
			if(ret == nullptr) {
				Console::Error("Failed to load library: %s", libName);
			}
			return ret;
		}

		void MakiFreeLibrary(void *lib)
		{
			if(lib == nullptr) {
				return;
			}
		#if defined(_WIN32) || defined(_WIN64)
			FreeLibrary((HMODULE)lib);
		#else

		#endif
		}

		void *MakiGetProcAddress(void *lib, const char *procName)
		{
			if(lib == nullptr) {
				return nullptr;
			}

			void *ret = nullptr;
		#if defined(_WIN32) || defined(_WIN64)
			ret = GetProcAddress((HMODULE)lib, procName);
		#else

		#endif
			return ret;
		}

	
	} // namespace Core

} // namespace Maki