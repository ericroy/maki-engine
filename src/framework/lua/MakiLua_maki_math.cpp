#include "framework/framework_stdafx.h"
#include "framework/lua/MakiLua_maki_math.h"
#include "framework/lua/MakiLua_maki.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{
		
			const char *udata_matrix44_t::type_name_ = "maki.math.Matrix44";

			int32 udata_matrix44_t::create(lua_State *L)
			{
				// First item on the stack is the self reference
				int32 arg_count = lua_gettop(L) - 1;

				matrix44_t m;
				switch(arg_count) {
				case 16:
					// All 16 values provided
					for(size_t i = 0; i < 4; i++) {
						m.cols_[i][0] = (float)luaL_checknumber(L, (i * 4) + 2);
						m.cols_[i][1] = (float)luaL_checknumber(L, (i * 4) + 3);
						m.cols_[i][2] = (float)luaL_checknumber(L, (i * 4) + 4);
						m.cols_[i][3] = (float)luaL_checknumber(L, (i * 4) + 5);
					}
					break;

				case 4:
					// Four vectors provided
#if _DEBUG
					luaL_checktype(L, 2, LUA_TTABLE);
					luaL_checktype(L, 3, LUA_TTABLE);
					luaL_checktype(L, 4, LUA_TTABLE);
					luaL_checktype(L, 5, LUA_TTABLE);
#endif
					for(size_t i = 0; i < 4; i++) {
						lua_rawgeti(L, 2 + i, 1);
						m.cols_[i][0] = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2 + i, 2);
						m.cols_[i][1] = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2 + i, 3);
						m.cols_[i][2] = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2 + i, 4);
						m.cols_[i][3] = (float)luaL_checknumber(L, -1);
					}
					break;

				case 1:
					switch(lua_type(L, 2)) {
					case LUA_TTABLE:
						lua_rawgeti(L, 2, 1);

						switch(lua_type(L, -1)) {
						case LUA_TTABLE:
							// A table of tables
							lua_rawgeti(L, 2, 2);
							lua_rawgeti(L, 2, 3);
							lua_rawgeti(L, 2, 4);
#if _DEBUG			
							luaL_checktype(L, 3, LUA_TTABLE);
							luaL_checktype(L, 4, LUA_TTABLE);
							luaL_checktype(L, 5, LUA_TTABLE);
							luaL_checktype(L, 6, LUA_TTABLE);
#endif
							for(size_t i = 0; i < 4; i++) {
								// Get column
								lua_rawgeti(L, 3 + i, 1);
								m.cols_[i][0] = (float)luaL_checknumber(L, -1);
								lua_rawgeti(L, 3 + i, 2);
								m.cols_[i][1] = (float)luaL_checknumber(L, -1);
								lua_rawgeti(L, 3 + i, 3);
								m.cols_[i][2] = (float)luaL_checknumber(L, -1);
								lua_rawgeti(L, 3 + i, 4);
								m.cols_[i][3] = (float)luaL_checknumber(L, -1);
							}
							break;

						case LUA_TNUMBER:
							// A table of numbers, all 16 values provided
							for(size_t i = 0; i < 16; i++) {
								lua_rawgeti(L, 2, i+1);
								m.vals_[i] = (float)luaL_checknumber(L, -1);
							}
							break;

						default:
							luaL_error(L, "Expected table of tables, or table of numbers");
							break;
						}
						
						break;

					case LUA_TUSERDATA:
						m = luaL_checkudata<udata_matrix44_t>(L, 2)->matrix_;
						break;

					default:
						luaL_typerror(L, 1, "table or maki.math.Matrix44");
						break;
					}
					break;

				case 0:
					m.set_identity();
					break;

				default:
					luaL_error(L, "Unexpected number of arguments");
					break;
				}

				lua_pop(L, lua_gettop(L));

				udata_matrix44_t *udata = luaL_pushudata<udata_matrix44_t>(L);
				udata->matrix_ = m;
				return 1;
			}

			int32 udata_matrix44_t::tostring(lua_State *L)
			{
				udata_matrix44_t *udata = luaL_toudata<udata_matrix44_t>(L, 1);
				lua_pushfstring(L, "<%s: [\n\t|%f, %f, %f, %f|\n\t|%f, %f, %f, %f|\n\t|%f, %f, %f, %f|\n\t|%f, %f, %f, %f|\n]>",
					udata->type_name_,
					udata->matrix_.cols_[0][0], udata->matrix_.cols_[1][0], udata->matrix_.cols_[2][0], udata->matrix_.cols_[3][0],
					udata->matrix_.cols_[0][1], udata->matrix_.cols_[1][1], udata->matrix_.cols_[2][1], udata->matrix_.cols_[3][1],
					udata->matrix_.cols_[0][2], udata->matrix_.cols_[1][2], udata->matrix_.cols_[2][2], udata->matrix_.cols_[3][2],
					udata->matrix_.cols_[0][3], udata->matrix_.cols_[1][3], udata->matrix_.cols_[2][3], udata->matrix_.cols_[3][3]
					);
				return 1;
			}

			const luaL_reg udata_matrix44_t::methods_[] = {
				{ nullptr, nullptr }
			};

			const luaL_reg udata_matrix44_t::metamethods_[] = {
				{ "__call", udata_matrix44_t::create },
				{ "__tostring", udata_matrix44_t::tostring },
				{ nullptr, nullptr }
			};



			const char *udata_vector4_t::type_name_ = "maki.math.Vector4";

			int32 udata_vector4_t::create(lua_State *L)
			{
				// First item on the stack is the self reference
				int32 arg_count = lua_gettop(L) - 1;

				vector4_t v;

				switch(arg_count) {
				case 4:
					// Loose values
					v.x_ = (float)luaL_checknumber(L, 2);
					v.y_ = (float)luaL_checknumber(L, 3);
					v.z_ = (float)luaL_checknumber(L, 4);
					v.w_ = (float)luaL_checknumber(L, 5);
					break;

				case 3:
					// Loose values
					v.x_ = (float)luaL_checknumber(L, 2);
					v.y_ = (float)luaL_checknumber(L, 3);
					v.z_ = (float)luaL_checknumber(L, 4);
					v.w_ = 1.0f;
					break;

				case 1:
					// A table of values or a single value to set as xyz
					switch(lua_type(L, 2)) {
					case LUA_TTABLE:
						lua_rawgeti(L, 2, 1);
						v.x_ = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2, 2);
						v.y_ = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2, 3);
						v.z_ = (float)luaL_checknumber(L, -1);
						lua_rawgeti(L, 2, 4);
						// W is optional
						v.w_ = (float)luaL_optnumber(L, -1, 1.0f);
						break;

					case LUA_TNUMBER:
						v.x_ = v.y_ = v.z_ = (float)lua_tonumber(L, 2);
						v.w_ = 1.0f;
						break;

					case LUA_TUSERDATA:
						v = luaL_checkudata<udata_vector4_t>(L, 2)->vector_;
						break;

					default:
						luaL_typerror(L, 1, "table, number, or maki.math.Vector4");
					}
					break;

				case 0:
					v = vector4_t(0.0f);
					break;

				default:
					luaL_error(L, "Unexpected number of arguments");
					break;
				}

				lua_pop(L, lua_gettop(L));

				udata_vector4_t *udata = luaL_pushudata<udata_vector4_t>(L);
				udata->vector_ = v;
				return 1;
			}

			int32 udata_vector4_t::tostring(lua_State *L)
			{
				udata_vector4_t *udata = luaL_toudata<udata_vector4_t>(L, 1);
				lua_pushfstring(L, "<%s: [%f, %f, %f, %f]>", udata->type_name_, udata->vector_.x_, udata->vector_.y_, udata->vector_.z_, udata->vector_.w_);
				return 1;
			}

			const luaL_reg udata_vector4_t::methods_[] = {
				{ nullptr, nullptr }
			};

			const luaL_reg udata_vector4_t::metamethods_[] = {
				{ "__call", udata_vector4_t::create },
				{ "__tostring", udata_vector4_t::tostring },
				{ nullptr, nullptr }
			};


			static const luaL_Reg module[] = {
				{ nullptr, nullptr }
			};

			LUALIB_API int32 luaopen_maki_math(lua_State *L)
			{
				luaL_register(L, "maki.math", module);

				lua_pushliteral(L, "Matrix44");
				register_type<udata_matrix44_t>(L);
				lua_rawset(L, -3);

				lua_pushliteral(L, "Vector4");
				register_type<udata_vector4_t>(L);
				lua_rawset(L, -3);

				return 0;
			}

		} // namespace lua
	} // namespace framework
} // namespace maki