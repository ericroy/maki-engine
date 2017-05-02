#include "core/MakiConsole.h"
#include <iostream>

#if MAKI_OS_WIN
#define sprintf_safe _vsprintf_p
#else
#define sprintf_safe vsnprintf
#endif

using namespace std;

namespace maki {
	namespace core {

		void console_t::write(level_t level, bool line_feed, const char *prefix, const char *format, va_list args) {
			if(level < verbosity_)
				return;

			char *cursor = buffer_;
			if(prefix != nullptr) {
				strcpy(buffer_, prefix);
				cursor += strlen(prefix);
			}

			sprintf_safe(cursor, max_buffer_size - (cursor - buffer_), format, args);
			
			if(line_feed)
				strcat(buffer_, "\n");

#if MAKI_CONSOLE_OUTPUT_ENABLED
			std::cout << buffer_;
			std::cout.flush();
#endif
			if(print_callback_ != nullptr)
				print_callback_(buffer_);

			if(level == level_die) {
#if MAKI_CONSOLE_OUTPUT_ENABLED
				std::cout << "exit(1)";
				std::cout.flush();
#endif
				if(print_callback_ != nullptr)
					print_callback_("exit(1)");
				exit(1);
			}
		}


	} // namespace core
} // namespace maki
