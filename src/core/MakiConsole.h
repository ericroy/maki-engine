#pragma once
#include <cstdarg>
#include <functional>
#include "core/MakiPseudoSingleton.h"

namespace maki {
	namespace core {
	
#define MAKI_DEFINE_CONSOLE_METHOD(METHOD_NAME, LEVEL, PREFIX, LINE_FEED) \
		static void METHOD_NAME(const char *format, ...) { \
			va_list args; \
			va_start(args, format); \
			console_t *c = get(); \
			if(c != nullptr) { \
				c->write(LEVEL, LINE_FEED, PREFIX, format, args); \
			} \
			va_end(args); \
		}

		

		class console_t : public pseudo_singleton_t<console_t> {
		public:
			enum level_t {
				level_info = 0,
				level_warning,
				level_error,
				level_die,
				level_max = level_die
			};
		
		public:
			static const int32_t max_buffer_size = 8192;
		
		public:
			MAKI_DEFINE_CONSOLE_METHOD(info, level_info,       "INFO  : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(lua, level_info,        "LUA   : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(warning, level_warning, "WARN  : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(lua_error, level_error, "LUAERR: ", true)
			MAKI_DEFINE_CONSOLE_METHOD(error, level_error,     "ERROR : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(die, level_die,         "DIE   : ", true)

			inline void set_verbosity(level_t level) { verbosity_ = level; }
			inline void set_print_callback(std::function<void(const char *)> func) { print_callback_ = func; }

		private:
			void write(level_t level, bool line_feed, const char *prefix, const char *format, va_list args);

		private:
			std::function<void(const char *)> print_callback_ = nullptr;
			char buffer_[max_buffer_size] = "";
			level_t verbosity_ = level_info;
		};

#undef MAKI_DEFINE_CONSOLE_METHOD

	} // namespace core
} // namespace maki
