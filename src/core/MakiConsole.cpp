#include "core/core_stdafx.h"
#include "core/MakiConsole.h"
#include <iostream>

#if defined(_MSC_VER)
#define sprintf_safe _vsprintf_p
#else
#define sprintf_safe vsnprintf
#endif


using namespace std;

namespace maki
{
	namespace core
	{

		console_t::console_t(level_t verbosity)
			: pseudo_singleton_t<console_t>(),
			verbosity(verbosity),
			printCallback(nullptr)
		{
			buffer[0] = 0;
		}

		console_t::~console_t()
		{
		}

		void console_t::write(level_t level, bool lineFeed, const char *prefix, const char *format, va_list args)
		{
			if(level < verbosity) {
				return;
			}

			char *cursor = buffer;
			if(prefix != nullptr) {
				strcpy(buffer, prefix);
				cursor += strlen(prefix);
			}

			sprintf_safe(cursor, max_buffer_size_ - (cursor - buffer), format, args);
			
			if(lineFeed) {
				strcat(buffer, "\n");
			}

#if MAKI_CONSOLE_OUTPUT_ENABLED
			std::cout << buffer;
			std::cout.flush();
#endif
			if(printCallback != nullptr) {
				printCallback(buffer);
			}

			if(level == Level_Die) {
#if MAKI_CONSOLE_OUTPUT_ENABLED
				std::cout << "exit(1)";
				std::cout.flush();
#endif
				if(printCallback != nullptr) {
					printCallback("exit(1)");
				}

				exit(1);
			}
		}


	} // namespace core

} // namespace maki
