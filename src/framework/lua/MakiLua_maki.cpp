#include "framework/framework_stdafx.h"
#include "framework/lua/MakiLua_maki.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{

			int32_t l_log(lua_State *L)
			{
				console_t::lua(luaL_checkstring(L, -1));
				return 0;
			}

			static const luaL_reg module[] = {
				{ "log", l_log },
				{ nullptr, nullptr }
			};

			LUALIB_API int32_t luaopen_maki(lua_State *L)
			{
				luaL_register(L, "maki", module);
				return 0;
			}

		} // namespace lua
	} // namespace framework
} // namespace maki