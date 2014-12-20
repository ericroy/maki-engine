#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"
#include <cstdarg>

namespace maki
{
	namespace core
	{
	
#define MAKI_DEFINE_CONSOLE_METHOD(METHOD_NAME, LEVEL, PREFIX, LINE_FEED) \
		static void METHOD_NAME(const char *format, ...) { \
			va_list args; \
			va_start(args, format); \
			console_t *c = Get(); \
			if(c != nullptr) { \
				c->write(LEVEL, LINE_FEED, PREFIX, format, args); \
			} \
			va_end(args); \
		}

		

		class console_t : public pseudo_singleton_t<console_t>
		{
		public:
			enum level_t
			{
				level_info_ = 0,
				level_warning_,
				level_error_,
				level_die_,
			};
		
		public:
			static const int32 max_buffer_size_ = 8192;
		
		public:
			MAKI_DEFINE_CONSOLE_METHOD(info, level_info_,       "INFO  : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(lua, level_info_,        "LUA   : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(warning, level_warning_, "WARN  : ", true)
			MAKI_DEFINE_CONSOLE_METHOD(lua_error, level_error_, "LUAERR: ", true)
			MAKI_DEFINE_CONSOLE_METHOD(error, level_error_,     "ERROR : ", true)			
			MAKI_DEFINE_CONSOLE_METHOD(die, level_die_,         "DIE   : ", true)
		public:
			console_t(level_t verbosity_ = level_info_);
			virtual ~console_t();

			inline void set_verbosity(level_t level) { verbosity_ = level; }
			inline void set_print_callback(std::function<void(const char *)> func) { print_callback_ = func; }

		private:
			void write(level_t level, bool lineFeed, const char *prefix, const char *format, va_list args);

		private:
			std::function<void(const char *)> print_callback_;
			char buffer_[max_buffer_size_];
			level_t verbosity_;
		};

#undef MAKI_DEFINE_CONSOLE_METHOD

	} // namespace core

} // namespace maki
