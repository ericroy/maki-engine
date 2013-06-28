#pragma once
#include "framework/framework_stdafx.h"
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
				script(HANDLE_NONE)
			{
			}

			Script::Script(HandleOrRid scriptId)
				: Component(TYPE, DEPENDENCIES),
				script(HANDLE_NONE)
			{
				//ResourceProvider *res = ResourceProvider::Get();

				if(scriptId.isHandle) {
					ScriptManager::AddRef(scriptId.handle);
					script = scriptId.handle;
				} else {
					//script = Load(scriptId.rid);
					if(script == HANDLE_NONE) {
						Console::Error("Script component failed to load script");
					}
				}
			}

			Script::~Script()
			{
				ScriptManager::Free(script);
			}

			bool Script::Init(Document::Node *props)
			{
				
				return true;
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki