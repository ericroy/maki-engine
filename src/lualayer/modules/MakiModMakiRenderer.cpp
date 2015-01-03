#include "lualayer/lualayer_stdafx.h"
#include <lua.hpp>
#include "core/MakiConsole.h"
#include "core/MakiEngine.h"
#include "core/MakiRect.h"

using namespace maki::core;

namespace maki
{
	namespace lualayer
	{
		namespace modules
		{

			void luaL_checkmatrix(lua_State *L, int32 index, matrix44_t *out)
			{
				if(lua_type(L, 1) == LUA_TNUMBER) {
					int32 count = lua_gettop(L) - index + 1;
					if(count < 16) {
						luaL_error(L, "Expected argument at index %d to be a matrix, but fewer than 16 values were specified", index);
					}
					out->vals_[0] = (float)lua_tonumber(L, 1);
					out->vals_[0] = (float)lua_tonumber(L, 2);
					out->vals_[0] = (float)lua_tonumber(L, 3);
					out->vals_[0] = (float)lua_tonumber(L, 4);
					out->vals_[0] = (float)lua_tonumber(L, 5);
					out->vals_[0] = (float)lua_tonumber(L, 6);
					out->vals_[0] = (float)lua_tonumber(L, 7);
					out->vals_[0] = (float)lua_tonumber(L, 8);
					out->vals_[0] = (float)lua_tonumber(L, 9);
					out->vals_[0] = (float)lua_tonumber(L, 10);
					out->vals_[0] = (float)lua_tonumber(L, 11);
					out->vals_[0] = (float)lua_tonumber(L, 12);
					out->vals_[0] = (float)lua_tonumber(L, 13);
					out->vals_[0] = (float)lua_tonumber(L, 14);
					out->vals_[0] = (float)lua_tonumber(L, 15);
					out->vals_[0] = (float)lua_tonumber(L, 16);
				} else {
					luaL_error(L, "Expected argument at index %d to be a matrix", index);
				}
			}


			int32 l_submit(lua_State *L)
			{
				engine_t::get()->renderer_->submit();
				return 0;
			}

			int32 l_set_viewport(lua_State *L)
			{
				float x = (float)luaL_optnumber(L, 1, 0.0f);
				float y = (float)luaL_optnumber(L, 2, 0.0f);
				float w = (float)luaL_optnumber(L, 3, 1.0f);
				float h = (float)luaL_optnumber(L, 4, 1.0f);
				engine_t::get()->renderer_->set_viewport(rect_t(x, y, y+h, x+w));
				return 0;
			}

			int32 l_clear_color(lua_State *L)
			{
				vector4_t color((float)luaL_optnumber(L, 1, 0.0f),
					(float)luaL_optnumber(L, 2, 0.0f),
					(float)luaL_optnumber(L, 3, 0.0f),
					(float)luaL_optnumber(L, 4, 1.0f));
				engine_t::get()->renderer_->clear_color(color);
				return 0;
			}

			int32 l_clear_depth(lua_State *L)
			{
				float depth = (float)luaL_optnumber(L, 1, 1.0f);
				engine_t::get()->renderer_->clear_depth(depth);
				return 0;
			}

			int32 l_set_camera_matrix(lua_State *L)
			{
				matrix44_t m;
				luaL_checkmatrix(L, 1, &m);
				engine_t::get()->renderer_->set_camera_matrix(m);
				return 0;
			}

			int32 l_set_perspective_projection(lua_State *L)
			{
				float w = (float)luaL_checknumber(L, 1);
				float h = (float)luaL_checknumber(L, 2);
				float n = (float)luaL_checknumber(L, 3);
				float f = (float)luaL_checknumber(L, 4);
				float fov = (float)luaL_optnumber(L, 1, 0);
				engine_t::get()->renderer_->set_perspective_projection(frustum_t(w, h, n, f, fov));
				return 0;
			}

			static const luaL_Reg module[] = {
				{ "set_viewport", l_set_viewport },
				{ "clear_color", l_clear_color },
				{ "clear_depth", l_clear_depth },
				{ "set_camera_matrix", l_set_camera_matrix },
				{ "set_perspective_projection", l_set_perspective_projection },
				{ "submit", l_submit },
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
