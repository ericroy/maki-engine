#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiScriptManager.h"
#include "framework/components/MakiScriptComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			Script::Script()
				: Component(TYPE, DEPENDENCIES),
				script(HANDLE_NONE),
				lastResult(0),
				sleepTime(0),
				coroutine(nullptr)
			{
			}

			Script::~Script()
			{
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
					ScriptManager::AddRef(scriptId.handle);
					script = scriptId.handle;
				} else {
					script = FrameworkManagers::Get()->scriptManager->Load(scriptId.rid);
					assert(script != HANDLE_NONE);
				}
				return true;
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki