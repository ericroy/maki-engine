#include "core/core_stdafx.h"
#include "core/MakiConsole.h"
#include <iostream>

#if defined(_MSC_VER)
#define sprintf_safe _vsprintf_p
#else
#define sprintf_safe vsnprintf
#endif


using namespace std;

namespace Maki
{
	namespace Core
	{

		Console::Console(Level verbosity)
			: PseudoSingleton<Console>(),
			verbosity(verbosity),
			printCallback(nullptr)
		{
			buffer[0] = 0;
		}

		Console::~Console()
		{
		}

		void Console::Write(Level level, bool lineFeed, const char *prefix, const char *format, va_list args)
		{
			if(level < verbosity) {
				return;
			}

			char *cursor = buffer;
			if(prefix != nullptr) {
				strcpy(buffer, prefix);
				cursor += strlen(prefix);
			}

			sprintf_safe(cursor, MAX_BUFFER_SIZE - (cursor - buffer), format, args);
			
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


	} // namespace Core

} // namespace Maki
