#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScriptingApi.h"
#include "framework/MakiSystem.h"
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
			{ "get_self", &GetSelf }
		};
		
		void ScriptingApi::ExposeApiToScript(Script *s)
		{
			lua_newtable(s->state);
			for(uint32 i = 0; i < sizeof(apiFunctions)/sizeof(ApiFunction); i++) {
				lua_pushstring(s->state, apiFunctions[i].name);
				lua_pushcfunction(s->state, apiFunctions[i].func);
				lua_settable(s->state, lua_gettop(s->state)-2);
			}
			lua_setglobal(s->state, "maki");
		}


		int32 ScriptingApi::PostMessage(lua_State *state)
		{
			int32 top = lua_gettop(state);
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, top-3);
			Component::Message msg = lua_tointeger(state, top-2);
			uintptr_t arg1 = (uintptr_t)lua_topointer(state, top-1);
			uintptr_t arg2 = (uintptr_t)lua_topointer(state, top);
			lua_settop(state, top-4);

			context->scriptSys->PostMessage(System::Message(context->scriptComp, msg, (void *)arg1, (void *)arg2));
			return 0;
		}

		int32 ScriptingApi::GetMessage(lua_State *state)
		{
			int32 top = lua_gettop(state);
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, top-1);
			int32 index = lua_tointeger(state, top);
			lua_settop(state, top-2);

			assert(index >= 0 && (uint32)index < context->scriptSys->currentlyProcessingQueue->size());
			const System::Message &m = (*context->scriptSys->currentlyProcessingQueue)[index];
			
			lua_pushlightuserdata(state, m.from);
			lua_pushinteger(state, (uint32)m.msg);
			lua_pushinteger(state, m.arg1);
			lua_pushinteger(state, m.arg2);
			return 4;
		}

		int32 ScriptingApi::GetSelf(lua_State *state)
		{
			int32 top = lua_gettop(state);
			ScriptingSystem::Node *context = (ScriptingSystem::Node *)lua_topointer(state, top);
			lua_settop(state, top-1);

			lua_pushlightuserdata(state, context->scriptComp);
			return 1;
		}

	} // namespace Core

} // namespace Maki