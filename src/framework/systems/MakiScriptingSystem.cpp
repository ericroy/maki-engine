#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiScriptManager.h"
#include "framework/MakiScript.h"
#include "framework/systems/MakiScriptingSystem.h"
#include "framework/components/MakiScriptComponent.h"
#include <sstream>

extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace Maki
{
	namespace Framework
	{
		namespace Systems
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



			ScriptingSystem::ScriptingSystem()
				: System(Component::TypeFlag_Script),
				state(nullptr)
			{
				state = luaL_newstate();
				assert(state != nullptr);

				lua_atpanic(state, LuaPanicHandler);
				luaL_openlibs(state);
				lua_register(state, "print", LuaPrint);
			}

			ScriptingSystem::~ScriptingSystem()
			{
				if(state != nullptr) {
					lua_close(state);
					state = nullptr;
				}
			}

			void ScriptingSystem::Update(float dt)
			{
				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					const Node &n = nodes[i];

					lua_getfield(state, LUA_GLOBALSINDEX, "update");
					lua_pushnumber(state, (double)dt);
					lua_call(state, 1, 0);
				}
			}

			void ScriptingSystem::Add(Entity *e)
			{
				Node n;
				n.scriptComp = e->Get<Components::Script>();
				n.valid = true;
				nodes.push_back(n);
				
				Components::Script *scriptComp = e->Get<Components::Script>();
				Framework::Script *s = FrameworkManagers::Get()->scriptManager->Get(scriptComp->script);
				
				// Evaluate the script
				if(luaL_loadstring(state, s->data) != 0 || lua_pcall(state, 0, LUA_MULTRET, 0) != 0) {
					LuaLogError();
					nodes.back().valid = false;
					return;	
				}

				// Ensure that the script exposes a function called 'update'
				lua_getfield(state, LUA_GLOBALSINDEX, "update");
				if(lua_type(state, 1) != LUA_TFUNCTION) {
					Console::Error("Script must expose a function called 'update'");
					nodes.back().valid = false;
				}
				LuaPop();				
			}

			void ScriptingSystem::Remove(Entity *e)
			{
				Node n;
				n.scriptComp = e->Get<Components::Script>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}


			void ScriptingSystem::LuaPop(int32 n)
			{
				lua_settop(state, -n-1);
			}

			void ScriptingSystem::LuaLogError()
			{
				const char *cs = luaL_checklstring(state, 1, nullptr);
				if(cs != nullptr) {
					Console::Error("LUA ERROR: %s", cs);
				} else {
					Console::Error("Error, could not get error message from lua stack");
				}
				LuaPop(1);
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki