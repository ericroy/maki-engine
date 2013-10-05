#pragma once
#include "framework/framework_stdafx.h"
#include "framework/hash/hash.h"
#include "framework/MakiScriptingApi.h"
#include "framework/MakiSystem.h"
#include "framework/MakiMessageHub.h"
#include "framework/MakiEntityPool.h"
#include "framework/systems/MakiScriptingSystem.h"
#include "framework/systems/MakiNameSystem.h"
#include "framework/components/MakiScriptComponent.h"
#include "framework/components/MakiMetaComponent.h"


namespace Maki
{
	namespace Framework
	{

		using namespace Systems;

		const ScriptingApi::ApiFunction ScriptingApi::apiFunctions[] = {
			{ "post_message", &PostMessage },
			{ "get_message", &GetMessage },
			{ "get_entity", &GetEntity },
			{ "get_property", &GetProperty },
			{ "set_property", &SetProperty },
			{ "set_message_handler", &SetMessageHandler },
			{ "hash", &Hash },
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

			if(!lua_isnumber(state, -4)) {
				lua_pushstring(state, "Failed to send message from lua, <to> argument must be a number");
				lua_error(state);
				return 0;
			}
			uint64 to = static_cast<uint64>(lua_tonumber(state, -4));
			
			uint32 msg = lua_tointeger(state, -3);
			uintptr_t arg1 = static_cast<uintptr_t>(lua_tonumber(state, -2));
			uintptr_t arg2 = static_cast<uintptr_t>(lua_tonumber(state, -1));
			
			lua_pop(state, 5);
			assert(lua_gettop(state) == 0);

			context->scriptSys->PostMessage(Message(context->scriptComp->owner->GetUid(), to, msg, arg1, arg2));
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
				lua_pushnumber(state, static_cast<double>(m->from));
				lua_pushnumber(state, static_cast<double>(m->to));
				lua_pushinteger(state, m->msg);
				lua_pushnumber(state, static_cast<double>(m->arg1));
				lua_pushnumber(state, static_cast<double>(m->arg2));
				return 5;
			} else {
				lua_pushnil(state);
				return 1;
			}			
		}

		int32 ScriptingApi::GetEntity(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -2);
			
			const char *entityName = lua_tostring(state, -1);
			Systems::NameSystem *nameSys = Systems::NameSystem::Get();
			Entity *ent = nameSys->GetEntityByName(entityName);
			
			lua_pop(state, 2);
			assert(lua_gettop(state) == 0);

			if(ent != nullptr) {
				lua_pushnumber(state, static_cast<double>(ent->GetUid()));
			} else {
				lua_pushnil(state);
			}			
			return 1;
		}

		int32 ScriptingApi::GetProperty(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -3);			
			uint64 uid = static_cast<uint64>(lua_tonumber(state, -2));
			const char *name = lua_tostring(state, -1);

			Components::Meta::Property *prop = nullptr;

			Entity *entity = EntityPool::Get()->GetEntity(uid);
			if(entity != nullptr) {
				Components::Meta *meta = entity->Get<Components::Meta>();
				if(meta != nullptr) {
					prop = meta->FindProperty(name);
				}
			}
		
			lua_pop(state, 3);
			assert(lua_gettop(state) == 0);

			if(prop != nullptr) {
				if(prop->type == Components::Meta::Property::Type_String) {
					lua_pushstring(state, prop->value.string);
					return 1;
				} else if(prop->type == Components::Meta::Property::Type_Number) {
					lua_pushnumber(state, prop->value.number);
					return 1;
				}
			}

			lua_pushnil(state);
			return 1;
		}

		int32 ScriptingApi::SetProperty(lua_State *state)
		{
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, -4);			
			uint64 uid = static_cast<uint64>(lua_tonumber(state, -3));
			const char *name = lua_tostring(state, -2);
			
			Entity *entity = EntityPool::Get()->GetEntity(uid);
			if(entity != nullptr) {
				Components::Meta *meta = entity->Get<Components::Meta>();
				if(meta != nullptr) {
					if(lua_isstring(state, -1)) {
						meta->SetProperty(name, lua_tostring(state, -1));
					} else if(lua_isnumber(state, -1)) {
						meta->SetProperty(name, (float)lua_tonumber(state, -1));
					} else if(lua_isnil(state, -1)) {
						meta->DeleteProperty(name);
					} else {
						lua_pushstring(state, "Cannot set meta property, invalid lua type");
						lua_error(state);
						return 0;
					}
				} else {
					char buffer[128];
					sprintf(buffer, "Cannot set meta property, entity had no meta component: %llu", uid);
					lua_pushstring(state, buffer);
					lua_error(state);
					return 0;
				}
			}

			lua_pop(state, 4);
			assert(lua_gettop(state) == 0);

			lua_pushnil(state);
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

		int32 ScriptingApi::Hash(lua_State *state)
		{
			const char *name = lua_tostring(state, -1);
			uint32 hash = Framework::Hash(name);
			lua_pop(state, 1);
			assert(lua_gettop(state) == 0);
			lua_pushinteger(state, hash);
			return 1;
		}

	} // namespace Framework

} // namespace Maki