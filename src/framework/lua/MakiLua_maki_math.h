#pragma once
#include "framework/framework_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace framework
	{
		namespace lua
		{
	
			struct udata_matrix44_t
			{
				static int32_t create(lua_State *L);
				static int32_t tostring(lua_State *L);

				static const luaL_reg methods_[];
				static const luaL_reg metamethods_[];
				static const char *type_name_;

				core::matrix44_t matrix_;
			};

			struct udata_vector4_t
			{
				static int32_t create(lua_State *L);
				static int32_t tostring(lua_State *L);

				static const luaL_reg methods_[];
				static const luaL_reg metamethods_[];
				static const char *type_name_;

				core::vector4_t vector_;
			};


			LUALIB_API int32_t luaopen_maki_math(lua_State *L);

		} // namespace lua

	} // namespace framework

} // namespace maki
