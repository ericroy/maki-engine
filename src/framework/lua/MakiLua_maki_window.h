#pragma once
#include "framework/framework_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace framework
	{
		namespace lua
		{
			
			LUALIB_API int32 luaopen_maki_window(lua_State *L);

		} // namespace lua

	} // namespace framework

} // namespace maki
