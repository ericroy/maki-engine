#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScriptingApi.h"
#include "framework/MakiSystem.h"
#include "framework/MakiMessageHub.h"
#include "framework/systems/MakiScriptingSystem.h"
#include "framework/components/MakiScriptComponent.h"

namespace Maki
{
	namespace Framework
	{

		using namespace Systems;

		const ScriptingApi::ApiFunction ScriptingApi::apiFunctions[] = {
			{ "post_message", &PostMessage },
			{ "get_message", &GetMessage },
			{ "get_self", &GetSelf },
			{ "set_message_handler", &SetMessageHandler },
		};
		
		void ScriptingApi::ExposeApiToScript(lua_State *state)
		{
			lua_newtable(state);
			for(uint32 i = 0; i < sizeof(apiFunctions)/sizeof(ApiFunction); i++) {
				lua_pushstring(state, apiFunctions[i].name);
				lua_pushcfunction(state, apiFunctions[i].func);
				lua_settable(state, lua_gettop(state)-2);
			}
			lua_setglobal(state, "maki");
		}


		int32 ScriptingApi::PostMessage(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -5);
			// TODO: Determine if this is handling 64 bit numbers properly
			uint64 to = static_cast<uint64>(lua_tonumber(state, -4));
			Component::Message msg = (Component::Message)lua_tointeger(state, -3);
			uintptr_t arg1 = (uintptr_t)lua_topointer(state, -2);
			uintptr_t arg2 = (uintptr_t)lua_topointer(state, -1);
			lua_pop(state, 5);
			assert(lua_gettop(state) == 0);

			context->scriptSys->PostMessage(Message(context->scriptComp->owner->GetUid(), to, msg, (void *)arg1, (void *)arg2));
			return 0;
		}

		int32 ScriptingApi::GetMessage(lua_State *state)
		{
			MessageHub *hub = MessageHub::Get();

			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -1);
			lua_pop(state, 1);
			assert(lua_gettop(state) == 0);
			
			// Find a message that is from an entity other than this one, and that is either a broadcast, or is addressed to this entity
			uint32 msgCount = hub->GetMessageCount();
			const Message *m = nullptr;

			uint64 thisUid = context->scriptComp->owner->GetUid();
			while(context->scriptComp->nextMessageIndex < msgCount) {
				const Message &msg = hub->GetMessages()[context->scriptComp->nextMessageIndex++];
				if(msg.from != thisUid && (msg.to == 0 || msg.to == thisUid)) {
					m = &msg;
					break;
				}
			}
			
			if(m != nullptr) {
				// TODO: Determine if this is handling 64 bit numbers properly
				lua_pushnumber(state, static_cast<double>(m->from));
				lua_pushnumber(state, static_cast<double>(m->to));
				lua_pushinteger(state, m->msg);
				lua_pushinteger(state, m->arg1);
				lua_pushinteger(state, m->arg2);
				return 5;
			} else {
				lua_pushnil(state);
				return 1;
			}			
		}

		int32 ScriptingApi::GetSelf(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -1);
			lua_pop(state, 1);
			assert(lua_gettop(state) == 0);

			lua_pushlightuserdata(state, context->scriptComp);
			return 1;
		}

		int32 ScriptingApi::SetMessageHandler(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -2);
			lua_remove(state, -2);

			// Message map function should be left on stack
			assert(lua_gettop(state) == 1);
			assert(lua_type(state, -1) == LUA_TFUNCTION);
			assert(context->scriptComp->messageHandler == nullptr && "Message handler func already registered");
			context->scriptComp->messageHandler = const_cast<void *>(lua_topointer(state, -1));


			// Store message handler func in registry for retrieval by the scripting system
			lua_pushlightuserdata(state, context->scriptComp->messageHandler);
			lua_insert(state, -2);
			lua_settable(state, LUA_REGISTRYINDEX);

			return 0;
		}

	} // namespace Core

} // namespace Maki