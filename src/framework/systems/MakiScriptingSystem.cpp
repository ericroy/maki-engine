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
	#include <luajit.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			static void LuaDumpStack(lua_State *state)
			{
				int32 top = lua_gettop(state);
				Console::Info("Lua stack (%d items):", top);
				for (int32 i = top; i >= 1; i--) {  // repeat for each level
					int t = lua_type(state, i);
					switch(t) {
					case LUA_TSTRING:  // strings
						Console::Info("[%d] string: '%s'", i, lua_tostring(state, i));
						break;

					case LUA_TBOOLEAN:  // booleans
						Console::Info("[%d] bool: %s", i, lua_toboolean(state, i) ? "true" : "false");
						break;

					case LUA_TNUMBER:  // numbers
						Console::Info("[%d] number: %g", i, lua_tonumber(state, i));
						break;

					default:  // other values
						Console::Info("[%d] %s: ???", i, lua_typename(state, t));
						break;
					}
				}
			}

			ScriptingSystem::ScriptingSystem()
				: System(Component::TypeFlag_Script),
				currentlyProcessingQueue(nullptr)
			{
			}

			ScriptingSystem::~ScriptingSystem()
			{
			}

			void ScriptingSystem::Update(float dt)
			{
				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					const Node &n = nodes[i];

					n.scriptComp->sleepTime -= dt;
					if(n.scriptComp->sleepTime > 0.0f) {
						continue;
					}
					n.scriptComp->sleepTime = 0.0f;

					int32 ret;
					if(n.scriptComp->lastResult == 0) {
						// Start a new coroutine
						lua_getglobal(n.scriptComp->coroutine, "run");
						lua_pushlightuserdata(n.scriptComp->coroutine, (void *)&n);
						ret = lua_resume(n.scriptComp->coroutine, 1);
					} else if(n.scriptComp->lastResult == LUA_YIELD) {
						// Resume a coroutine
						ret = lua_resume(n.scriptComp->coroutine, 0);
					} else {
						// Script has died, don't resume it
						continue;
					}

					if(ret == LUA_YIELD) {
						// Update function yielded - get the number of seconds to sleep this coroutine before starting it again
						n.scriptComp->sleepTime = (float)lua_tonumber(n.scriptComp->coroutine, -1);
						lua_pop(n.scriptComp->coroutine, 1);
					} else if(ret == 0) {
						// Update function returned, we'll start it again on the next update
					} else {
						Console::Error("LUA ERROR: %s", lua_tolstring(n.scriptComp->coroutine, -1, nullptr));
						LuaDumpStack(n.scriptComp->coroutine);
						lua_pop(n.scriptComp->coroutine, 1);
						n.scriptComp->sleepTime = 0.0f;
					}

					n.scriptComp->lastResult = ret;
				}
			}

			void ScriptingSystem::ProcessMessages(const std::vector<Message> &messages)
			{
				currentlyProcessingQueue = &messages;

				const uint32 messageCount = messages.size();
				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					const Node &n = nodes[i];

					if(n.scriptComp->handlesMessages) {
						lua_getfield(n.scriptComp->state, LUA_GLOBALSINDEX, "message_handler");
						lua_pushlightuserdata(n.scriptComp->state, (void *)&n);
						lua_pushinteger(n.scriptComp->state, messageCount);
						lua_pcall(n.scriptComp->state, 2, 0, 0);
					}
				}

				currentlyProcessingQueue = nullptr;
			}

			void ScriptingSystem::Add(Entity *e)
			{
				Components::Script *scriptComp = e->Get<Components::Script>();

				Node n;
				n.scriptSys = this;
				n.scriptComp = scriptComp;
				nodes.push_back(n);
				
				Framework::Script *s = FrameworkManagers::Get()->scriptManager->Get(scriptComp->script);
				scriptComp->coroutine = lua_newthread(s->state);
				if(scriptComp->coroutine == nullptr) {
					Console::Error("Failed to create new lua thread for coroutine");
					return;
				}

				// Ensure that the script exposes an init function, taking an entity, returning a coroutine
				lua_getfield(s->state, LUA_GLOBALSINDEX, "run");
				if(lua_type(s->state, -1) != LUA_TFUNCTION) {
					Console::Error("Script must expose a function with signature run(entity) that will be executed as a coroutine");
				}
				lua_pop(s->state, 1);

				lua_getfield(s->state, LUA_GLOBALSINDEX, "message_handler");
				scriptComp->handlesMessages = lua_type(s->state, -1) == LUA_TFUNCTION;				
				lua_pop(s->state, 1);

				scriptComp->lastResult = 0;	// LUA_OK
			}

			void ScriptingSystem::Remove(Entity *e)
			{
				// According to the lua_newthread docs, threads are garbage collected and I don't have to explicitly destroy it here
				
				Node n;
				n.scriptComp = e->Get<Components::Script>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}


			

		} // namespace Systems

	} // namespace Framework

} // namespace Maki