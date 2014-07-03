#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

#include <lua.hpp>


namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{
			class ScriptingSystem;
		}

		namespace Components
		{

			class Script : public Component
			{
				friend class Systems::ScriptingSystem;
				friend class ScriptingApi;

			public:
				static const Type TYPE = Type_Script;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Script();
				virtual ~Script();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid scriptId);
				Script *Clone(bool prototype);

			private:
				Handle script;

				// The main lua context for the script handle above (we keep a pointer here for efficiency)
				lua_State *state;

				int32 lastResult;
				float sleepTime;
				
				// Lua message handling function (or null if one is not registered)
				void *messageHandler;

				// When processing messages, this bookmarks the message that should be returned when 
				// the script asks for the next message
				uint32 nextMessageIndex;
				
				// The execution context for the coroutine that was launched via run
				// Shares state with the main context belonging to the script
				lua_State *coroutine;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki