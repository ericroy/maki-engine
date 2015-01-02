#include "lualayer/lualayer_stdafx.h"
#include "lualayer/MakiLuaContext.h"
#include "core/MakiConsole.h"
#include "core/MakiEngine.h"

using namespace maki::core;

namespace maki
{
	namespace lualayer
	{

		namespace modules
		{

			int32 l_test(lua_State *L)
			{
				const char *s = lua_tostring(L, -1);
				console_t::lua("Lua says: %s", s);
				return 0;
			}

			static const luaL_Reg module[] = {
				{ "test", l_test },
				{ nullptr, nullptr }
			};

			LUALIB_API int32 luaopen_maki(lua_State *L)
			{
				luaL_register(L, "maki", module);
				return 0;
			}

		} // namespace modules

	} // namespace lualayer

} // namespace maki
