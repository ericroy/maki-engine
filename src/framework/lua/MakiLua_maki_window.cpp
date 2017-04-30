#include "framework/framework_stdafx.h"
#include "framework/lua/MakiLua_maki_window.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{

			int32_t l_get_width(lua_State *L)
			{
				lua_pushnumber(L, engine_t::get()->window_->width_);
				return 1;
			}

			int32_t l_get_height(lua_State *L)
			{
				lua_pushnumber(L, engine_t::get()->window_->height_);
				return 1;
			}

			int32_t l_get_dimensions(lua_State *L)
			{
				lua_pushnumber(L, engine_t::get()->window_->width_);
				lua_pushnumber(L, engine_t::get()->window_->height_);
				return 2;
			}

			static const luaL_Reg module[] = {
				{ "get_width", l_get_width },
				{ "get_height", l_get_height },
				{ "get_dimensions", l_get_dimensions },
				{ nullptr, nullptr }
			};

			LUALIB_API int32_t luaopen_maki_window(lua_State *L)
			{
				luaL_register(L, "maki.window", module);
				return 0;
			}

		} // namespace lua
	} // namespace framework
} // namespace maki
