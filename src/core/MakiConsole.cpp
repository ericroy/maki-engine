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
			verbosity_(verbosity),
			print_callback_(nullptr)
		{
			buffer_[0] = 0;
		}

		console_t::~console_t()
		{
		}

		void console_t::write(level_t level, bool lineFeed, const char *prefix, const char *format, va_list args)
		{
			if(level < verbosity_) {
				return;
			}

			char *cursor = buffer_;
			if(prefix != nullptr) {
				strcpy(buffer_, prefix);
				cursor += strlen(prefix);
			}

			sprintf_safe(cursor, max_buffer_size_ - (cursor - buffer_), format, args);
			
			if(lineFeed) {
				strcat(buffer_, "\n");
			}

#if MAKI_CONSOLE_OUTPUT_ENABLED
			std::cout << buffer_;
			std::cout.flush();
#endif
			if(print_callback_ != nullptr) {
				print_callback_(buffer_);
			}

			if(level == level_die_) {
#if MAKI_CONSOLE_OUTPUT_ENABLED
				std::cout << "exit(1)";
				std::cout.flush();
#endif
				if(print_callback_ != nullptr) {
					print_callback_("exit(1)");
				}

				exit(1);
			}
		}


	} // namespace core

} // namespace maki
