#pragma once
#include "framework/framework_stdafx.h"

extern "C"
{
	#include <luajit.h>
}

namespace Maki
{
	namespace Framework
	{

		class ScriptUtils
		{
		public:

			// See http://medek.wordpress.com/2009/02/03/wrapping-lua-errors-and-print-function/
			static int32 LuaPrint(lua_State *state);

			static int32 LuaPanicHandler(lua_State *state);
			static int32 LuaLoaderFunc(lua_State *state);
			static int32 LuaSearcherFunc(lua_State *state);

			static void DumpLuaStack(lua_State *state);
		};

	} // namespace Core

} // namespace Maki