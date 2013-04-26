#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class Console
	{
	public:
		enum Level
		{
			Level_Info = 0,
			Level_Warning,
			Level_Error,
			Level_None,
		};
		
		static std::function<void(Level, char *)> PrintfCallback;

	private:
		static const int32 MAX_BUFFER_SIZE = 8192;
		static char buffer[MAX_BUFFER_SIZE];
		static Level verbosity;
		
	public:
		static void SetVerbosity(Level level) { verbosity = level; }

		static void Info(const char *format, ...);
		static void Warning(const char *format, ...);
		static void Error(const char *format, ...);

		static void InfoNoLineFeed(const char *format, ...);
		static void WarningNoLineFeed(const char *format, ...);
		static void ErrorNoLineFeed(const char *format, ...);
	};

} // namespace Maki