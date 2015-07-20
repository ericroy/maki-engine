#pragma once
#include "framework/framework_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace framework
	{
		namespace lua
		{

			struct udata_handle_t
			{
				typedef void(*handle_free_proc_t)(handle_t &);

				static int32 tostring(lua_State *L);
				static int32 gc(lua_State *L);

				static const luaL_reg methods_[];
				static const luaL_reg metamethods_[];
				static const char *type_name_;

				handle_t handle_;
				handle_free_proc_t free_proc_;
			};

			LUALIB_API int32 luaopen_maki_assets(lua_State *L);

		} // namespace lua

	} // namespace framework

} // namespace maki
