#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScriptUtils.h"
#include "framework/MakiScript.h"
#include <sstream>

extern "C"
{
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace Maki
{
	namespace Framework
	{

		// See http://medek.wordpress.com/2009/02/03/wrapping-lua-errors-and-print-function/
		int32 ScriptUtils::LuaPrint(lua_State *state)
		{
			int32 argCount = lua_gettop(state);
			lua_getglobal(state, "tostring");
				
			std::stringstream buffer;
				
			// Make sure you start at 1 *NOT* 0
			for(int32 i = 1; i <= argCount; i++)
			{
				lua_pushvalue(state, -1);
				lua_pushvalue(state, i);
				lua_call(state, 1, 1);
				const char *s = lua_tostring(state, -1);
				if(s == nullptr) {
					return luaL_error(state, "'tostring' must return a string to print");
				}
				if(i > 1) {
					buffer << "\t";
				}
				buffer << s;
				lua_pop(state, 1);
			};
			Console::Lua(buffer.str().c_str());
			return 0;
		}

		int32 ScriptUtils::LuaPanicHandler(lua_State *state)
		{
			const char *cs = luaL_checklstring(state, 1, nullptr);
			Console::LuaError("PANIC: %s", cs);
			lua_pop(state, 1);
			return 0;
		}

		int32 ScriptUtils::LuaLoaderFunc(lua_State *state)
		{
			std::string moduleName = lua_tostring(state, -1);
			lua_pop(state, 1);

			std::map<std::string, Rid>::iterator iter = Script::modules.find(moduleName);
			assert(iter != Script::modules.end());
				
			uint32 bytesRead;
			char *data = Engine::Get()->assets->AllocRead(iter->second, &bytesRead);
			if(data == nullptr) {
				Console::LuaError("Failed to load lua script with Rid<%u>", iter->second);
				goto failed;
			}

			int32 ret = luaL_loadbuffer(state, data, bytesRead, moduleName.c_str());
			if(ret != 0) {
				Console::LuaError(luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;
			}

			ret = lua_pcall(state, 0, 1, 0);
			if(ret != 0) {
				Console::LuaError(luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;
			}
			
			SAFE_FREE(data);
			return 1;

		failed:
			SAFE_FREE(data);
			return 0;
		}

		int32 ScriptUtils::LuaSearcherFunc(lua_State *state)
		{
			const char *moduleName = lua_tostring(state, -1);
			std::map<std::string, Rid>::iterator iter = Script::modules.find(moduleName);
			if(iter != Script::modules.end()) {
				lua_pushcfunction(state, LuaLoaderFunc);
				return 1;
			}
			return 0;
		}


		void ScriptUtils::DumpLuaStack(lua_State *state)
		{
			int32 top = lua_gettop(state);
			Console::Lua("Lua stack (%d items):", top);
			for (int32 i = top; i >= 1; i--) {  // repeat for each level
				int t = lua_type(state, i);
				switch(t) {
				case LUA_TSTRING:  // strings
					Console::Lua("[%d] string: '%s'", i, lua_tostring(state, i));
					break;

				case LUA_TBOOLEAN:  // booleans
					Console::Lua("[%d] bool: %s", i, lua_toboolean(state, i) ? "true" : "false");
					break;

				case LUA_TNUMBER:  // numbers
					Console::Lua("[%d] number: %g", i, lua_tonumber(state, i));
					break;

				default:  // other values
					Console::Lua("[%d] %s: ???", i, lua_typename(state, t));
					break;
				}
			}
		}


	} // namespace Core

} // namespace Maki