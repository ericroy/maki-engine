#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScript.h"
#include "framework/MakiScriptUtils.h"
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

			lua_atpanic(state, ScriptUtils::LuaPanicHandler);
			luaL_openlibs(state);
			lua_register(state, "print", ScriptUtils::LuaPrint);
			ScriptingApi::ExposeApiToScript(this->state);

			// Register custom searcher so that one script can 'require' another
            lua_getglobal(state, "package");
			assert(!lua_isnil(state, -1));
			lua_getfield(state, -1, "loaders");
			assert(!lua_isnil(state, -1));

			int32 loaderCount = lua_objlen(state, -1);
            lua_pushinteger(state, loaderCount + 1);
            lua_pushcfunction(state, ScriptUtils::LuaSearcherFunc);
            lua_settable(state, -3);
            lua_pop(state, 2);

			// Load and evaluate the script
			char chunkName[32];
			sprintf_s(chunkName, "Rid<%u>", scriptRid);

			if(luaL_loadbuffer(state, data, bytesRead, chunkName) != 0) {
				Console::LuaError(luaL_checklstring(state, 1, nullptr));
				lua_pop(state, 1);
				goto failed;				
			}

			if(lua_pcall(state, 0, 0, 0) != 0) {
				Console::LuaError(luaL_checklstring(state, 1, nullptr));
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