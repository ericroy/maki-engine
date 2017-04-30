#pragma once
#include "framework/framework_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace framework
	{
		namespace lua
		{
			
			LUALIB_API int32_t luaopen_maki_renderer(lua_State *L);

		} // namespace modules

	} // namespace framework

} // namespace maki
