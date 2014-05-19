#pragma once
#include "framework/framework_stdafx.h"

#include <lua.hpp>

namespace Maki
{
	namespace Framework
	{

		class ScriptingApi
		{
		public:
			static void ExposeApiToScript(lua_State *state);

		private:
			static int32 PostMessage(lua_State *state);
			static int32 GetMessage(lua_State *state);
			static int32 GetEntity(lua_State *state);
			static int32 GetProperty(lua_State *state);
			static int32 SetProperty(lua_State *state);
			static int32 SetMessageHandler(lua_State *state);
			static int32 Hash(lua_State *state);

		private:
			static const luaL_Reg apiFunctions[];
		};


	} // namespace Core

} // namespace Maki