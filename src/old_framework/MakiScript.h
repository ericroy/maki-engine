#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiResource.h"
#include <lua.hpp>

namespace Maki
{
	namespace Framework
	{

		class Script : public Resource
		{
		public:
			static std::map<std::string, Rid> modules;

		public:
			Script();
			virtual ~Script();
			bool Load(Rid scriptRid);
			void PrintLuaError();

		private:
			Script(const Script &other) {}

		public:
			char *data;
			lua_State *state;
		};

	} // namespace Core

} // namespace Maki