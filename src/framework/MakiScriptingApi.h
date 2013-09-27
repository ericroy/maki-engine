#pragma once
#include "framework/framework_stdafx.h"

extern "C"
{
	#include <luajit.h>
}

namespace Maki
{
	namespace Framework
	{

		class ScriptingApi
		{
		private:
			enum MetaType
			{
				MetaType_String = 0,
				MetaType_Number
			};

			struct  ApiFunction { const char *name; lua_CFunction func; };

		public:
			static void ExposeApiToScript(lua_State *state);

		private:
			static int32 PostMessage(lua_State *state);
			static int32 GetMessage(lua_State *state);
			static int32 GetEntity(lua_State *state);
			static int32 GetMeta(lua_State *state);
			static int32 SetMessageHandler(lua_State *state);

		private:
			static const ApiFunction apiFunctions[];
		};


	} // namespace Core

} // namespace Maki