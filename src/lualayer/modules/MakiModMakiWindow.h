#pragma once
#include "lualayer/lualayer_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace lualayer
	{
		namespace modules
		{

			LUALIB_API int32 luaopen_maki_window(lua_State *L);

		} // namespace modules

	} // namespace lualayer

} // namespace maki
