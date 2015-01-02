#pragma once
#include "lualayer/lualayer_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace lualayer
	{
		namespace modules
		{
			extern int32 lua_update_function_ref;
			extern int32 lua_draw_function_ref;

			LUALIB_API int32 luaopen_maki(lua_State *L);

		} // namespace modules

	} // namespace lualayer

} // namespace maki
