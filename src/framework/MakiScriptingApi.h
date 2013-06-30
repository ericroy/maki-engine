#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScript.h"

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
			struct ApiFunction { const char *name; lua_CFunction func; };

		public:
			static void ExposeApiToScript(Script *s);

		private:
			static int32 PostMessage(lua_State *state);
			static int32 GetMessage(lua_State *state);
			static int32 GetSelf(lua_State *state);

		private:
			static const ApiFunction apiFunctions[];
		};


	} // namespace Core

} // namespace Maki