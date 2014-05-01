#pragma once
#include "core/core_stdafx.h"
#include "core/MakiPseudoSingleton.h"
#include <cstdarg>

namespace Maki
{
	namespace Core
	{
	
#define _MAKI_DEFINE_CONSOLE_METHOD(_methodName, _level, _prefix, _lineFeed) \
		static void _methodName(const char *format, ...) { \
			va_list args; \
			va_start(args, format); \
			Console *c = Get(); \
			if(c != nullptr) { \
				c->Write(_level, _lineFeed, _prefix, format, args); \
			} \
			va_end(args); \
		}

		

		class Console : public PseudoSingleton<Console>
		{
		public:
			enum Level
			{
				Level_Info = 0,
				Level_Warning,
				Level_Error,
				Level_Die,
			};
		
		public:
			static const int32 MAX_BUFFER_SIZE = 8192;
		
		public:
			_MAKI_DEFINE_CONSOLE_METHOD(Info,		Level_Info,		"INFO  : ", true)
			_MAKI_DEFINE_CONSOLE_METHOD(Lua,		Level_Info,		"LUA   : ", true)
			_MAKI_DEFINE_CONSOLE_METHOD(Warning,	Level_Warning,	"WARN  : ", true)
			_MAKI_DEFINE_CONSOLE_METHOD(LuaError,	Level_Error,	"LUAERR: ", true)
			_MAKI_DEFINE_CONSOLE_METHOD(Error,		Level_Error,	"ERROR : ", true)			
			_MAKI_DEFINE_CONSOLE_METHOD(Die,		Level_Die,		"DIE   : ", true)

		public:
			Console(Level verbosity = Level_Info);
			virtual ~Console();

			inline void SetVerbosity(Level level) { verbosity = level; }
			inline void SetPrintCallback(std::function<void(const char *)> func) { printCallback = func; }

		private:
			void Write(Level level, bool lineFeed, const char *prefix, const char *format, va_list args);

		private:
			std::function<void(const char *)> printCallback;
			char buffer[MAX_BUFFER_SIZE];
			Level verbosity;
		};

#undef _MAKI_DEFINE_CONSOLE_METHOD

	} // namespace Core

} // namespace Maki