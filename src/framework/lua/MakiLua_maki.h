#pragma once
#include "framework/framework_stdafx.h"
#include <lua.hpp>

namespace maki
{
	namespace framework
	{
		namespace lua
		{

			template<class T>
			T *luaL_pushudata(lua_State *L)
			{
				T *data = static_cast<T *>(lua_newuserdata(L, sizeof(T)));
				luaL_getmetatable(L, T::type_name_);
				lua_setmetatable(L, -2);
				return data;
			}

			template<class T>
			T *luaL_toudata(lua_State *L, int index)
			{
#if _DEBUG
				T *data = static_cast<T *>(luaL_checkudata(L, index, T::type_name_));
#else
				T *data = static_cast<T *>(lua_touserdata(L, index));
#endif
				return data;
			}

			template<class T>
			T *luaL_checkudata(lua_State *L, int index)
			{
#if _DEBUG
				T *data = static_cast<T *>(luaL_checkudata(L, index, T::type_name_));
#else
				T *data = static_cast<T *>(lua_touserdata(L, index));
				if(data == nullptr) {
					luaL_typerror(L, index, T::type_name_);
				}
#endif
				return data;
			}


			template<class T>
			int32_t register_type(lua_State *L)
			{
				luaL_openlib(L, T::type_name_, T::methods_, 0); // [methods]

				luaL_newmetatable(L, T::type_name_); // [methods, metatable]
				luaL_openlib(L, 0, T::metamethods_, 0); // [methods, metatable]
				

				lua_pushliteral(L, "__index"); // [methods, metatable, "__index"]
				// dup methods table
				lua_pushvalue(L, -3); // [methods, metatable, "__index", methods]				
				// metatable.__index = methods
				lua_rawset(L, -3); // [methods, metatable]
				

				lua_pushliteral(L, "__metatable"); // [methods, metatable, "__metatable"]
				// dup methods table
				lua_pushvalue(L, -3); // [methods, metatable, "__metatable", methods]
				// hide metatable: metatable.__metatable = methods
				lua_rawset(L, -3); // [methods, metatable]


				// setmetatable(methods, metatable)
				lua_setmetatable(L, -2); // [methods, metatable]

				// drop metatable
				//lua_pop(L, 1); // [methods]

				return 1;  // return methods on the stack
			}
			
			LUALIB_API int32_t luaopen_maki(lua_State *L);

		} // namespace lua

	} // namespace framework

} // namespace maki
