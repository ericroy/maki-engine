#include "lualayer/lualayer_stdafx.h"
#include <lua.hpp>
#include "core/MakiConsole.h"
#include "core/MakiEngine.h"

using namespace maki::core;

namespace maki
{
	namespace lualayer
	{
		namespace modules
		{
			int32 l_get_width(lua_State *L)
			{
				lua_pushnumber(L, engine_t::get()->window_->width_);
				return 1;
			}

			int32 l_get_height(lua_State *L)
			{
				lua_pushnumber(L, engine_t::get()->window_->height_);
				return 1;
			}

			int32 l_get_dimensions(lua_State *L)
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

			LUALIB_API int32 luaopen_maki_window(lua_State *L)
			{
				luaL_register(L, "maki.window", module);
				return 0;
			}

		} // namespace modules

	} // namespace lualayer

} // namespace maki
