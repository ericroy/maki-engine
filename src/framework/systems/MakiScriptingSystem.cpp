#include "framework/framework_stdafx.h"
#include "framework/systems/MakiScriptingSystem.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiScriptManager.h"
#include "framework/MakiScriptUtils.h"
#include "framework/MakiScript.h"
#include "framework/MakiMessageHub.h"
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

			ScriptingSystem::ScriptingSystem(uint32 messageQueueSize)
				: SystemBase(Component::TypeFlag_Script, 0, messageQueueSize, "ScriptingSystem")
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
					switch(n.scriptComp->lastResult) {
					case -1:
						// Script not started yet, kick off a new coroutine
						lua_getglobal(n.scriptComp->coroutine, "run");
						lua_pushlightuserdata(n.scriptComp->coroutine, (void *)&n);
						ret = lua_resume(n.scriptComp->coroutine, 1);
						break;
					case LUA_YIELD:
						// Resume a coroutine
						ret = lua_resume(n.scriptComp->coroutine, 0);
						break;
					case 0:
						// Script finished - do not restart
						continue;
					default:
						// Anything else is an error - script has died.
						// Do not restart it.
						continue;
					}

					if(ret == LUA_YIELD) {
						// Update function yielded - get the number of seconds to sleep this coroutine before starting it again
						n.scriptComp->sleepTime = (float)lua_tonumber(n.scriptComp->coroutine, -1);
						lua_pop(n.scriptComp->coroutine, 1);
					} else if(ret == 0) {
						// Script finished
					} else {
						Console::LuaError(lua_tolstring(n.scriptComp->coroutine, -1, nullptr));
						ScriptUtils::DumpLuaStack(n.scriptComp->coroutine);
						lua_pop(n.scriptComp->coroutine, 1);
						n.scriptComp->sleepTime = 0.0f;
					}

					n.scriptComp->lastResult = ret;
				}
			}

			void ScriptingSystem::ProcessMessages()
			{
				const uint32 messageCount = MessageHub::Get()->GetMessageCount();
				if(messageCount == 0) {
					return;
				}

				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					const Node &n = nodes[i];
					lua_State *state = n.scriptComp->state;

					if(n.scriptComp->messageHandler != nullptr) {
						n.scriptComp->nextMessageIndex = 0;

						// Load the message handler func onto the top of the stack
						lua_pushlightuserdata(state, n.scriptComp->messageHandler);
						lua_gettable(state, LUA_REGISTRYINDEX);

						// Push the args and call it
						lua_pushlightuserdata(state, (void *)&n);
						lua_pcall(state, 1, 0, 0);
					}
				}
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

				// Store the thread in the lua registry so it doesn't get garbage collected
				lua_pushlightuserdata(s->state, scriptComp->coroutine);
				lua_insert(s->state, -2);
				lua_settable(s->state, LUA_REGISTRYINDEX);

				// Ensure that the script exposes an init function, taking an entity, returning a coroutine
				lua_getfield(s->state, LUA_GLOBALSINDEX, "run");
				if(lua_type(s->state, -1) != LUA_TFUNCTION) {
					Console::Error("Script must expose a function with signature run(entity) that will be executed as a coroutine");
				}
				lua_pop(s->state, 1);

				// Indicate that script has not been started yet
				scriptComp->lastResult = -1;
			}

			void ScriptingSystem::Remove(Entity *e)
			{
				Components::Script *scriptComp = e->Get<Components::Script>();
				Framework::Script *s = FrameworkManagers::Get()->scriptManager->Get(scriptComp->script);

				// Remove the message handler func from the registry (if there is one)
				if(scriptComp->messageHandler != nullptr) {
					lua_pushlightuserdata(s->state, scriptComp->messageHandler);
					lua_pushnil(s->state);
					lua_settable(s->state, LUA_REGISTRYINDEX);
					scriptComp->messageHandler = nullptr;
				}

				// Remove thread from the lua registry so it can get garbage collected
				lua_pushlightuserdata(s->state, scriptComp->coroutine);
				lua_pushnil(s->state);
				lua_settable(s->state, LUA_REGISTRYINDEX);
				scriptComp->coroutine = nullptr;
				
				Node n;
				n.scriptComp = scriptComp;
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}


			

		} // namespace Systems

	} // namespace Framework

} // namespace Maki