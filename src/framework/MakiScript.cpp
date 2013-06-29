#pragma once
#include "framework/framework_stdafx.h"
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
		static int LuaPrint(lua_State *state)
		{
			int argCount = lua_gettop(state);
			lua_getglobal(state, "tostring");
				
			std::stringstream buffer("LUA: ");
				
			// Make sure you start at 1 *NOT* 0
			for(int i = 1; i <= argCount; i++)
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
			if(cs != nullptr) {
				Console::Error("LUA PANIC: %s", cs);
			} else {
				Console::Error("LUA PANIC: (No error available)");
			}
			lua_settop(state, -2);
			return 0;
		}




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

		static const char *readerFunc(lua_State *state, void *data, size_t *size)
		{
			std::pair<char *, uint32> &chunk = *(std::pair<char *, uint32> *)data;
			*size = chunk.second;
			return chunk.first;
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

			char chunkName[64];
			sprintf_s(chunkName, "Rid<%u>", scriptRid);

			std::pair<char *, uint32> chunk(data, bytesRead);

			// Load and evaluate the script
			if(lua_load(state, readerFunc, &chunk, chunkName) != 0 || lua_pcall(state, 0, LUA_MULTRET, 0) != 0) {
				const char *cs = luaL_checklstring(state, 1, nullptr);
				if(cs != nullptr) {
					Console::Error("LUA ERROR: %s", cs);
				} else {
					Console::Error("Error, could not get error message from lua stack");
				}
				lua_settop(state, -2);

				SAFE_FREE(data);
				return false;
			}

			SAFE_FREE(data);
			this->rid = scriptRid;
			return true;
		}


	} // namespace Core

} // namespace Maki