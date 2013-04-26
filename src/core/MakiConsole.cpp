#include "core/core_stdafx.h"
#include "core/MakiConsole.h"
#include <iostream>
#include <cstdarg>


#if defined(_WIN32) || defined(_WIN64)
#define sprintf_safe vsnprintf_s
#else
#define sprintf_safe vsnprintf
#endif


using namespace std;

namespace Maki
{

	char Console::buffer[] = "";
	std::function<void(Console::Level, char *)> Console::PrintfCallback = nullptr;
	Console::Level Console::verbosity = Console::Level_Info;


	void Console::Info(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Info < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE-1, format, argList);
		strcat_s(buffer, "\n");
		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Info, buffer);
		}

		va_end(argList);
#endif
	}

	void Console::Warning(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Warning < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE-1, format, argList);
		strcat_s(buffer, "\n");
		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Warning, buffer);
		}

		va_end(argList);
#endif
	}

	void Console::Error(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Error < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE-1, format, argList);
		strcat_s(buffer, "\n");
		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Error, buffer);
		}

		va_end(argList);
#endif
	}

	void Console::InfoNoLineFeed(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Info < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE, format, argList);

		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Info, buffer);
		}

		va_end(argList);
#endif
	}

	void Console::WarningNoLineFeed(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Warning < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE, format, argList);

		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Warning, buffer);
		}

		va_end(argList);
#endif
	}

	void Console::ErrorNoLineFeed(const char *format, ...)
	{
#if MAKI_CONSOLE_OUTPUT_ENABLED
		if(Level_Error < verbosity) {
			return;
		}

		va_list argList;
		va_start(argList, format);

		sprintf_safe(buffer, MAX_BUFFER_SIZE, format, argList);

		cout << buffer;

		if(PrintfCallback != nullptr) {
			PrintfCallback(Level_Error, buffer);
		}

		va_end(argList);
#endif
	}


} // namespace Maki
