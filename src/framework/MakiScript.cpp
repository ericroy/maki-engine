#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScript.h"
#include "framework/MakiScriptingApi.h"
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
		static int LuaPrint(lua_State *state)
		{
			int32 argCount = lua_gettop(state);
			lua_getglobal(state, "tostring");
				
			std::stringstream buffer("LUA: ");
				
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
			Console::Info(buffer.str().c_str());
			return 0;
		}

		static int LuaPanicHandler(lua_State *state)
		{
			const char *cs = luaL_checklstring(state, 1, nullptr);
			Console::Error("LUA PANIC: %s", cs);
			lua_pop(state, 1);
			return 0;
		}

		static int32 LuaLoaderFunc(lua_State *state)
		{
			std::string moduleName = lua_tostring(state, -1);
			lua_pop(state, 1);

			std::map<std::string, Rid>::iterator iter = Script::modules.find(moduleName);
			assert(iter != Script::modules.end());
				
			uint32 bytesRead;
			char *data = Engine::Get()->assets->AllocRead(iter->second, &bytesRead);
			if(data == nullptr) {
				Console::Error("Failed to load lua script with Rid<%u>", iter->second);
				goto failed;
			}

			int32 ret = luaL_loadbuffer(state, data, bytesRead, moduleName.c_str());
			if(ret != 0) {
				Console::Error("LUA ERROR: %s", luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;
			}

			ret = lua_pcall(state, 0, 1, 0);
			if(ret != 0) {
				Console::Error("LUA ERROR: %s", luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;
			}
			
			SAFE_FREE(data);
			return 1;

		failed:
			SAFE_FREE(data);
			return 0;
		}

		static int32 LuaSearcherFunc(lua_State *state)
		{
			const char *moduleName = lua_tostring(state, -1);
			std::map<std::string, Rid>::iterator iter = Script::modules.find(moduleName);
			if(iter != Script::modules.end()) {
				lua_pushcfunction(state, LuaLoaderFunc);
				return 1;
			}
			return 0;
		}



		





		std::map<std::string, Rid> Script::modules;

		Script::Script()
			: Resource(),
			data(nullptr),
			state(nullptr)
		{
		}

		Script::~Script()
		{
			if(state != nullptr) {
				lua_close(state);
				state = nullptr;
			}
		}

		bool Script::Load(Rid scriptRid)
		{
			uint32 bytesRead;
			data = Engine::Get()->assets->AllocRead(scriptRid, &bytesRead);

			state = luaL_newstate();
			assert(state != nullptr);

			lua_atpanic(state, LuaPanicHandler);
			luaL_openlibs(state);
			lua_register(state, "print", LuaPrint);
			ScriptingApi::ExposeApiToScript(this->state);

			// Register custom searcher so that one script can 'require' another
            lua_getglobal(state, "package");
			assert(!lua_isnil(state, -1));
			lua_getfield(state, -1, "loaders");
			assert(!lua_isnil(state, -1));

			int32 loaderCount = lua_objlen(state, -1);
            lua_pushinteger(state, loaderCount + 1);
            lua_pushcfunction(state, LuaSearcherFunc);
            lua_settable(state, -3);
            lua_pop(state, 2);

			// Load and evaluate the script
			char chunkName[32];
			sprintf_s(chunkName, "Rid<%u>", scriptRid);

			if(luaL_loadbuffer(state, data, bytesRead, chunkName) != 0) {
				Console::Error("LUA ERROR: %s", luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;				
			}

			if(lua_pcall(state, 0, 0, 0) != 0) {
				Console::Error("LUA ERROR: %s", luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;
			}

			SAFE_FREE(data);
			this->rid = scriptRid;
			return true;

		failed:
			SAFE_FREE(data);
			return false;
		}


	} // namespace Core

} // namespace Maki