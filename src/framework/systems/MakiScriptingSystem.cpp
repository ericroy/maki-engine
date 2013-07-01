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

					if(n.scriptComp->lastResult == 0) {
						// Start a new coroutine
						lua_getglobal(n.scriptComp->coroutine, "run");
						lua_pushlightuserdata(n.scriptComp->coroutine, (void *)&n);
						n.scriptComp->lastResult = lua_resume(n.scriptComp->coroutine, 1);
					} else if(n.scriptComp->lastResult == LUA_YIELD) {
						// Resume a coroutine
						n.scriptComp->lastResult = lua_resume(n.scriptComp->coroutine, 0);
					}

					if(n.scriptComp->lastResult == LUA_YIELD) {
						// Update function yielded - get the number of seconds to sleep this coroutine before starting it again
						n.scriptComp->sleepTime = (float)lua_tonumber(n.scriptComp->coroutine, -1);
					} else if(n.scriptComp->lastResult == 0) {
						// Update function returned, we'll start it again on the next update
					} else {
						Console::Error("LUA ERROR: %s", lua_tolstring(n.scriptComp->coroutine, -1, nullptr));
						lua_pop(n.scriptComp->coroutine, 1);
						n.scriptComp->lastResult = 0;
						n.scriptComp->sleepTime = 0.0f;
					}
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
						lua_getfield(n.scriptComp->coroutine, LUA_GLOBALSINDEX, "message_handler");
						lua_pushlightuserdata(n.scriptComp->coroutine, (void *)&n);
						lua_pushinteger(n.scriptComp->coroutine, messageCount);
						lua_pcall(n.scriptComp->coroutine, 2, 0, 0);
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