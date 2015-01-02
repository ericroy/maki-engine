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

			int32 l_begin(lua_State *L)
			{
				bool ok = engine_t::get()->renderer_->begin();
				lua_pushboolean(L, ok ? 1 : 0);
				return 1;
			}

			int32 l_submit(lua_State *L)
			{
				engine_t::get()->renderer_->submit();
				return 0;
			}

			int32 l_end(lua_State *L)
			{
				engine_t::get()->renderer_->end();
				return 0;
			}

			static const luaL_Reg module[] = {
				{ "begin", l_begin },
				{ "submit", l_submit },
				{ "end", l_end },
				{ nullptr, nullptr }
			};

			LUALIB_API int32 luaopen_maki_renderer(lua_State *L)
			{
				luaL_register(L, "maki.renderer", module);
				return 0;
			}

		} // namespace modules

	} // namespace lualayer

} // namespace maki
