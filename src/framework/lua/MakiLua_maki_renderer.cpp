#include "framework/framework_stdafx.h"
#include "framework/lua/MakiLua_maki_renderer.h"
#include "framework/lua/MakiLua_maki.h"
#include "framework/lua/MakiLua_maki_math.h"
#include "framework/lua/MakiLua_maki_assets.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{
			
			int32_t l_submit(lua_State *L)
			{
				engine_t::get()->renderer_->submit();
				return 0;
			}

			int32_t l_set_viewport(lua_State *L)
			{
				float x = (float)luaL_optnumber(L, 1, 0.0f);
				float y = (float)luaL_optnumber(L, 2, 0.0f);
				float w = (float)luaL_optnumber(L, 3, 1.0f);
				float h = (float)luaL_optnumber(L, 4, 1.0f);
				engine_t::get()->renderer_->set_viewport(rect_t(x, y, y + h, x + w));
				return 0;
			}

			int32_t l_clear_color(lua_State *L)
			{
				vector4_t color((float)luaL_optnumber(L, 1, 0.0f),
					(float)luaL_optnumber(L, 2, 0.0f),
					(float)luaL_optnumber(L, 3, 0.0f),
					(float)luaL_optnumber(L, 4, 1.0f));
				engine_t::get()->renderer_->clear_color(color);
				return 0;
			}

			int32_t l_clear_depth(lua_State *L)
			{
				float depth = (float)luaL_optnumber(L, 1, 1.0f);
				engine_t::get()->renderer_->clear_depth(depth);
				return 0;
			}

			int32_t l_set_camera_matrix(lua_State *L)
			{
				udata_matrix44_t *m = luaL_checkudata<udata_matrix44_t>(L, 1);
				engine_t::get()->renderer_->set_camera_matrix(m->matrix_);
				return 0;
			}

			int32_t l_get_camera_matrix(lua_State *L)
			{
				const matrix44_t &m = engine_t::get()->renderer_->get_camera_matrix();
				lua_newtable(L);
				for(int32_t i = 0; i < 16; i++) {
					lua_pushnumber(L, m.vals_[i]);
					lua_rawseti(L, -2, i + 1);
				}
				return 1;
			}

			int32_t l_set_view(lua_State *L)
			{
				udata_matrix44_t *m = luaL_checkudata<udata_matrix44_t>(L, 1);
				engine_t::get()->renderer_->set_view(m->matrix_);
				return 0;
			}

			int32_t l_get_view(lua_State *L)
			{
				const matrix44_t &m = engine_t::get()->renderer_->get_view();
				lua_newtable(L);
				for(int32_t i = 0; i < 16; i++) {
					lua_pushnumber(L, m.vals_[i]);
					lua_rawseti(L, -2, i + 1);
				}
				return 1;
			}

			int32_t l_set_ortho_projection(lua_State *L)
			{
				float w = (float)luaL_checknumber(L, 1);
				float h = (float)luaL_checknumber(L, 2);
				float n = (float)luaL_checknumber(L, 3);
				float f = (float)luaL_checknumber(L, 4);
				engine_t::get()->renderer_->set_ortho_projection(frustum_t(w, h, n, f));
				return 0;
			}

			int32_t l_set_perspective_projection(lua_State *L)
			{
				float w = (float)luaL_checknumber(L, 1);
				float h = (float)luaL_checknumber(L, 2);
				float n = (float)luaL_checknumber(L, 3);
				float f = (float)luaL_checknumber(L, 4);
				float fov = (float)luaL_optnumber(L, 1, 0);
				engine_t::get()->renderer_->set_perspective_projection(frustum_t(w, h, n, f, fov));
				return 0;
			}

			int32_t l_set_render_target(lua_State *L)
			{
				const char *render_target_type = luaL_checkstring(L, 1);

				handle_t render_target = HANDLE_NONE;
				if(lua_islightuserdata(L, 2)) {
					udata_handle_t *data = luaL_toudata<udata_handle_t>(L, 2);
					if(data != nullptr) {
						render_target = data->handle_;
					}
				}

				render_state_t::render_target_t rt = render_state_t::render_target_null_;
				if(strcmp(render_target_type, "null") == 0) {
					rt = render_state_t::render_target_null_;
				} else if(strcmp(render_target_type, "default") == 0) {
					rt = render_state_t::render_target_default_;
				} else if(strcmp(render_target_type, "custom")) {
					rt = render_state_t::render_target_custom_;
				} else {
					luaL_error(L, "Invalid render target type: %s", render_target_type);
				}

				engine_t::get()->renderer_->set_render_target(rt, render_target);
				return 0;
			}

			int32_t l_set_depth_stencil(lua_State *L)
			{
				const char *depth_stencil_type = luaL_checkstring(L, 1);

				handle_t depth_stencil = HANDLE_NONE;
				if(lua_islightuserdata(L, 2)) {
					udata_handle_t *data = static_cast<udata_handle_t *>(lua_touserdata(L, 2));
					if(data != nullptr) {
						depth_stencil = data->handle_;
					}
				}

				render_state_t::depth_stencil_t ds = render_state_t::depth_stencil_null_;
				if(strcmp(depth_stencil_type, "null") == 0) {
					ds = render_state_t::depth_stencil_null_;
				} else if(strcmp(depth_stencil_type, "default") == 0) {
					ds = render_state_t::depth_stencil_default_;
				} else if(strcmp(depth_stencil_type, "custom")) {
					ds = render_state_t::depth_stencil_custom_;
				} else {
					luaL_error(L, "Invalid depth stencil type: %s", depth_stencil_type);
				}

				engine_t::get()->renderer_->set_depth_stencil(ds, depth_stencil);
				return 0;
			}

			int32_t l_set_cull_mode(lua_State *L)
			{
				const char *cull_mode = luaL_checkstring(L, 1);
				render_state_t::cull_mode_t cm = render_state_t::cull_mode_none_;
				if(strcmp(cull_mode, "back") == 0) {
					cm = render_state_t::cull_mode_back_;
				} else if(strcmp(cull_mode, "front") == 0) {
					cm = render_state_t::cull_mode_front_;
				} else if(strcmp(cull_mode, "none") == 0) {
					cm = render_state_t::cull_mode_none_;
				} else {
					luaL_error(L, "Invalid cull mode: %s", cull_mode);
				}

				engine_t::get()->renderer_->set_cull_mode(cm);
				return 0;
			}

			int32_t l_get_cull_mode(lua_State *L)
			{
				render_state_t::cull_mode_t cm = engine_t::get()->renderer_->get_cull_mode();
				switch(cm) {
				case render_state_t::cull_mode_front_:
					lua_pushliteral(L, "front");
					break;
				case render_state_t::cull_mode_back_:
					lua_pushliteral(L, "back");
					break;
				case render_state_t::cull_mode_none_:
					lua_pushliteral(L, "none");
					break;
				default:
					assert(false);
				}
				return 1;
			}

			int32_t l_set_wire_frame_enabled(lua_State *L)
			{
				bool on = lua_toboolean(L, 1) != 0 ? true : false;
				engine_t::get()->renderer_->set_wire_frame_enabled(on);
				return 0;
			}

			int32_t l_get_wire_frame_enabled(lua_State *L)
			{
				bool on = engine_t::get()->renderer_->get_wire_frame_enabled();
				lua_pushboolean(L, on ? 1 : 0);
				return 1;
			}

			int32_t l_set_depth_write_enabled(lua_State *L)
			{
				bool on = lua_toboolean(L, 1) != 0 ? true : false;
				engine_t::get()->renderer_->set_depth_write_enabled(on);
				return 0;
			}

			int32_t l_get_depth_write_enabled(lua_State *L)
			{
				bool on = engine_t::get()->renderer_->get_depth_write_enabled();
				lua_pushboolean(L, on ? 1 : 0);
				return 1;
			}

			int32_t l_set_depth_test(lua_State *L)
			{
				const char *depth_test = luaL_checkstring(L, 1);
				render_state_t::depth_test_t dt = render_state_t::depth_test_disabled_;
				if(strcmp(depth_test, "less") == 0) {
					dt = render_state_t::depth_test_less_;
				} else if(strcmp(depth_test, "equal") == 0) {
					dt = render_state_t::depth_test_equal_;
				} else if(strcmp(depth_test, "less_equal") == 0) {
					dt = render_state_t::depth_test_less_equal_;
				} else if(strcmp(depth_test, "disabled") == 0) {
					dt = render_state_t::depth_test_disabled_;
				} else {
					luaL_error(L, "Invalid depth test: %s", depth_test);
				}

				engine_t::get()->renderer_->set_depth_test(dt);
				return 0;
			}

			int32_t l_get_depth_test(lua_State *L)
			{
				render_state_t::depth_test_t dt = engine_t::get()->renderer_->get_depth_test();
				switch(dt) {
				case render_state_t::depth_test_less_:
					lua_pushliteral(L, "less");
					break;
				case render_state_t::depth_test_equal_:
					lua_pushliteral(L, "equal");
					break;
				case render_state_t::depth_test_less_equal_:
					lua_pushliteral(L, "less_equal");
					break;
				case render_state_t::depth_test_disabled_:
					lua_pushliteral(L, "disabled");
					break;
				default:
					assert(false);
				}
				return 1;
			}

			int32_t l_set_global_ambient_color(lua_State *L)
			{
				vector4_t color((float)luaL_optnumber(L, 1, 0.0f),
					(float)luaL_optnumber(L, 2, 0.0f),
					(float)luaL_optnumber(L, 3, 0.0f),
					(float)luaL_optnumber(L, 4, 1.0f));
				engine_t::get()->renderer_->set_global_ambient_color(color);
				return 0;
			}

			int32_t l_get_global_ambient_color(lua_State *L)
			{
				const vector4_t &c = engine_t::get()->renderer_->get_global_ambient_color();
				lua_pushnumber(L, c.x_);
				lua_pushnumber(L, c.y_);
				lua_pushnumber(L, c.z_);
				lua_pushnumber(L, c.w_);
				return 4;
			}

			int32_t l_set_shader_variant(lua_State *L)
			{
				const char *variant = luaL_checkstring(L, 1);
				shader_program_t::variant_t v = shader_program_t::variant_normal_;
				if(strcmp(variant, "normal") == 0) {
					v = shader_program_t::variant_normal_;
				} else if(strcmp(variant, "depth") == 0) {
					v = shader_program_t::variant_depth_;
				} else if(strcmp(variant, "shadow") == 0) {
					v = shader_program_t::variant_shadow_;
				} else {
					luaL_error(L, "Invalid shader variant: %s", variant);
				}
				engine_t::get()->renderer_->set_shader_variant(v);
				return 0;
			}

			int32_t l_get_shader_variant(lua_State *L)
			{
				shader_program_t::variant_t v = engine_t::get()->renderer_->get_shader_variant();
				switch(v) {
				case shader_program_t::variant_normal_:
					lua_pushliteral(L, "normal");
					break;
				case shader_program_t::variant_depth_:
					lua_pushliteral(L, "depth");
					break;
				case shader_program_t::variant_shadow_:
					lua_pushliteral(L, "shadow");
					break;
				default:
					assert(false);
				}
				return 1;
			}

			static const luaL_Reg module[] = {
				{ "set_viewport", l_set_viewport },
				{ "clear_color", l_clear_color },
				{ "clear_depth", l_clear_depth },
				{ "set_view", l_set_view },
				{ "get_view", l_get_view },
				{ "set_camera_matrix", l_set_camera_matrix },
				{ "get_camera_matrix", l_get_camera_matrix },
				{ "set_ortho_projection", l_set_ortho_projection },
				{ "set_perspective_projection", l_set_perspective_projection },
				{ "set_render_target", l_set_render_target },
				{ "set_depth_stencil", l_set_depth_stencil },
				{ "set_cull_mode", l_set_cull_mode },
				{ "get_cull_mode", l_get_cull_mode },
				{ "set_wire_frame_enabled", l_set_wire_frame_enabled },
				{ "get_wire_frame_enabled", l_get_wire_frame_enabled },
				{ "set_depth_write_enabled", l_set_depth_write_enabled },
				{ "get_depth_write_enabled", l_get_depth_write_enabled },
				{ "set_depth_test", l_set_depth_test },
				{ "get_depth_test", l_get_depth_test },
				{ "set_global_ambient_color", l_set_global_ambient_color },
				{ "get_global_ambient_color", l_get_global_ambient_color },
				{ "set_shader_variant", l_set_shader_variant },
				{ "get_shader_variant", l_get_shader_variant },
				{ "submit", l_submit },
				{ nullptr, nullptr }
			};

			LUALIB_API int32_t luaopen_maki_renderer(lua_State *L)
			{
				luaL_register(L, "maki.renderer", module);
				return 0;
			}

		} // namespace lua
	} // namespace framework
} // namespace maki