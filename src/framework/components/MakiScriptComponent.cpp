#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiScriptComponent.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiScriptManager.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			Script::Script()
				: Component(TYPE, DEPENDENCIES),
				script(HANDLE_NONE),
				lastResult(-1),
				sleepTime(0),
				state(nullptr),
				coroutine(nullptr),
				messageHandler(nullptr)
			{
			}

			Script::~Script()
			{
				state = nullptr;
				ScriptManager::Free(script);
			}

			bool Script::Init(Document::Node *props)
			{
				const char *scriptPath = props->ResolveValue("script.#0");
				if(scriptPath == nullptr) {
					Console::Error("Entity did not specify a script");
					return false;
				}
				Rid scriptRid = Engine::Get()->assets->PathToRid(scriptPath);
				if(scriptRid == RID_NONE) {
					Console::Error("No RID for path: %s", scriptRid);
					return false;
				}

				return Init(scriptRid);
			}

			bool Script::Init(HandleOrRid scriptId)
			{
				if(scriptId.isHandle) {
					if(scriptId.handle == HANDLE_NONE) {
						Console::Error("Script component script handle cannot be HANDLE_NONE");
						return false;
					}
					ScriptManager::AddRef(scriptId.handle);
					script = scriptId.handle;
				} else {
					script = FrameworkManagers::Get()->scriptManager->Load(scriptId.rid);
					if(script == HANDLE_NONE) {
						Console::Error("Script component failed to load script with Rid<%u>", scriptId.rid);
						return false;
					}
				}

				state = ScriptManager::Get(script)->state;
				return true;
			}

			Script *Script::Clone(bool prototype)
			{
				Script *c = ComponentPool<Script>::Get()->Create();

				ScriptManager::AddRef(script);
				c->script = script;
				c->state = state;

				return c;
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki