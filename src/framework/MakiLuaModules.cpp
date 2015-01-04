#include "framework/framework_stdafx.h"
#include "framework/MakiLuaModules.h"
#include <lua.hpp>
#include "core/MakiConsole.h"
#include "core/MakiEngine.h"
#include "core/MakiManager.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{

			typedef void(*handle_free_proc_t)(handle_t &);

			static void free_texture_handle(handle_t &h) { manager_t<mesh_t, mesh_manager_t>::free(h); }
			static void free_texture_set_handle(handle_t &h) { manager_t<texture_set_t, texture_set_manager_t>::free(h); }
			static void free_material_handle(handle_t &h) { manager_t<material_t, material_manager_t>::free(h); }
			static void free_mesh_handle(handle_t &h) { manager_t<mesh_t, mesh_manager_t>::free(h); }
			static void free_vertex_format_handle(handle_t &h) { manager_t<vertex_format_t, vertex_format_manager_t>::free(h); }
			static void free_shader_program_handle(handle_t &h) { manager_t<shader_program_t, shader_program_manager_t>::free(h); }
			static void free_font_handle(handle_t &h) { manager_t<font_t, font_manager_t>::free(h); }
			static void free_skeleton_handle(handle_t &h) { manager_t<skeleton_t, skeleton_manager_t>::free(h); }
			static void free_skeleton_animation_handle(handle_t &h) { manager_t<skeleton_animation_t, skeleton_animation_manager_t>::free(h); }
			

			void luaL_checkmatrix(lua_State *L, int32 index, matrix44_t *out)
			{
#if _DEBUG
				luaL_checktype(L, index, LUA_TTABLE);
#endif
				for(int32 i = 0; i < 16; i++) {
					lua_rawgeti(L, index, i + 1);
					out->vals_[i] = (float)lua_tonumber(L, -1);
				}
				lua_pop(L, 16);
			}

			struct udata_handle_t
			{
				static const char *type_name_;
				handle_t handle_;
				handle_free_proc_t free_proc_;
			};

			const char *udata_handle_t::type_name_ = "maki.Handle";

			udata_handle_t *luaL_pushhandle(lua_State *L)
			{
				udata_handle_t *data = static_cast<udata_handle_t *>(lua_newuserdata(L, sizeof(udata_handle_t)));
				luaL_getmetatable(L, udata_handle_t::type_name_);
				lua_setmetatable(L, -2);
				return data;
			}

			udata_handle_t *luaL_tohandle(lua_State *L, int index)
			{
#if _DEBUG
				udata_handle_t *data = static_cast<udata_handle_t *>(luaL_checkudata(L, index, udata_handle_t::type_name_));
#else
				udata_handle_t *data = static_cast<udata_handle_t *>(lua_touserdata(L, index));
				if(data == nullptr) {
					luaL_typerror(L, index, udata_handle_t::type_name_);
				}
#endif
				return data;
			}



			namespace maki
			{
				int32 l_handle_gc(lua_State *L)
				{
					udata_handle_t *data = luaL_tohandle(L, 1);
					data->free_proc_(data->handle_);
					return 0;
				}

				int32 l_handle_tostring(lua_State *L)
				{
					lua_pushfstring(L, "Handle: %p", lua_touserdata(L, 1));
					return 0;
				}

				static const luaL_Reg handle_methods[] = {
						{ "free", l_handle_gc },
						{ nullptr, nullptr }
				};

				static const luaL_reg handle_metamethods[] = {
						{ "__gc", l_handle_gc },
						{ "__tostring", l_handle_tostring },
						{ nullptr, nullptr }
				};

				int32 register_handle(lua_State *L)
				{
					luaL_openlib(L, udata_handle_t::type_name_, handle_methods, 0);
					luaL_newmetatable(L, udata_handle_t::type_name_);
					luaL_openlib(L, 0, handle_metamethods, 0);
					lua_pushliteral(L, "__index");
					lua_pushvalue(L, -3);  // dup methods table
					lua_rawset(L, -3);  // metatable.__index = methods
					lua_pushliteral(L, "__metatable");
					lua_pushvalue(L, -3);  // dup methods table
					lua_rawset(L, -3);  // hide metatable: metatable.__metatable = methods
					lua_pop(L, 1);  // drop metatable
					return 1;  // return methods on the stack
				}

				static const luaL_Reg module[] = {
						{ nullptr, nullptr }
				};

				LUALIB_API int32 luaopen_maki(lua_State *L)
				{
					luaL_register(L, "maki", module);

					// Register maki.Handle
					lua_pushliteral(L, "Handle");
					register_handle(L);
					lua_rawset(L, -3);
					return 0;
				}
			}


			namespace renderer
			{
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
					engine_t::get()->renderer_->set_viewport(rect_t(x, y, y + h, x + w));
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

				int32 l_get_camera_matrix(lua_State *L)
				{
					const matrix44_t &m = engine_t::get()->renderer_->get_camera_matrix();
					lua_newtable(L);
					for(int32 i = 0; i < 16; i++) {
						lua_pushnumber(L, m.vals_[i]);
						lua_rawseti(L, -2, i+1);
					}
					return 1;
				}

				int32 l_set_view(lua_State *L)
				{
					matrix44_t m;
					luaL_checkmatrix(L, 1, &m);
					engine_t::get()->renderer_->set_view(m);
					return 0;
				}

				int32 l_get_view(lua_State *L)
				{
					const matrix44_t &m = engine_t::get()->renderer_->get_view();
					lua_newtable(L);
					for(int32 i = 0; i < 16; i++) {
						lua_pushnumber(L, m.vals_[i]);
						lua_rawseti(L, -2, i+1);
					}
					return 1;
				}

				int32 l_set_ortho_projection(lua_State *L)
				{
					float w = (float)luaL_checknumber(L, 1);
					float h = (float)luaL_checknumber(L, 2);
					float n = (float)luaL_checknumber(L, 3);
					float f = (float)luaL_checknumber(L, 4);
					engine_t::get()->renderer_->set_ortho_projection(frustum_t(w, h, n, f));
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

				int32 l_set_render_target(lua_State *L)
				{
					const char *render_target_type = luaL_checkstring(L, 1);

					handle_t render_target = HANDLE_NONE;
					if(lua_islightuserdata(L, 2)) {
						udata_handle_t *data = static_cast<udata_handle_t *>(lua_touserdata(L, 2));
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

				int32 l_set_depth_stencil(lua_State *L)
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

				int32 l_set_cull_mode(lua_State *L)
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

				int32 l_get_cull_mode(lua_State *L)
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

				int32 l_set_wire_frame_enabled(lua_State *L)
				{
					bool on = lua_toboolean(L, 1) != 0 ? true : false;
					engine_t::get()->renderer_->set_wire_frame_enabled(on);
					return 0;
				}

				int32 l_get_wire_frame_enabled(lua_State *L)
				{
					bool on = engine_t::get()->renderer_->get_wire_frame_enabled();
					lua_pushboolean(L, on ? 1 : 0);
					return 1;
				}

				int32 l_set_depth_write_enabled(lua_State *L)
				{
					bool on = lua_toboolean(L, 1) != 0 ? true : false;
					engine_t::get()->renderer_->set_depth_write_enabled(on);
					return 0;
				}

				int32 l_get_depth_write_enabled(lua_State *L)
				{
					bool on = engine_t::get()->renderer_->get_depth_write_enabled();
					lua_pushboolean(L, on ? 1 : 0);
					return 1;
				}

				int32 l_set_depth_test(lua_State *L)
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

				int32 l_get_depth_test(lua_State *L)
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

				int32 l_set_global_ambient_color(lua_State *L)
				{
					vector4_t color((float)luaL_optnumber(L, 1, 0.0f),
						(float)luaL_optnumber(L, 2, 0.0f),
						(float)luaL_optnumber(L, 3, 0.0f),
						(float)luaL_optnumber(L, 4, 1.0f));
					engine_t::get()->renderer_->set_global_ambient_color(color);
					return 0;
				}

				int32 l_get_global_ambient_color(lua_State *L)
				{
					const vector4_t &c = engine_t::get()->renderer_->get_global_ambient_color();
					lua_pushnumber(L, c.x_);
					lua_pushnumber(L, c.y_);
					lua_pushnumber(L, c.z_);
					lua_pushnumber(L, c.w_);
					return 4;
				}

				int32 l_set_shader_variant(lua_State *L)
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

				int32 l_get_shader_variant(lua_State *L)
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

				LUALIB_API int32 luaopen_maki_renderer(lua_State *L)
				{
					luaL_register(L, "maki.renderer", module);
					return 0;
				}
			}


			namespace window
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
			}

			namespace assets
			{
				int32 l_load_texture(lua_State *L)
				{
					const char *path = luaL_checkstring(L, 1);
					rid_t rid = engine_t::get()->assets_->path_to_rid(path);
					if(rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find texture for path: %s", path);
						return 2;
					}
					handle_t h = core_managers_t::get()->texture_manager_->load(rid);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_texture_handle;
					return 1;
				}

				int32 l_load_texture_set(lua_State *L)
				{
					rid_t rids[texture_set_t::max_textures_per_set_];
					int32 count = lua_gettop(L);
					if(lua_type(L, 1) == LUA_TSTRING) {
						for(int32 i = 0; i < count; i++) {
							const char *path = luaL_checkstring(L, i + 1);
							rid_t rid = engine_t::get()->assets_->path_to_rid(path);
							if(rid == RID_NONE) {
								lua_pushnil(L);
								lua_pushfstring(L, "Could not find texture for path: %s", path);
								return 2;
							}
							rids[i] = rid;
						}
					} else {
						for(int32 i = 0; i < count; i++) {
							const char *path = luaL_checkstring(L, i + 1);
							rids[i] = luaL_checkinteger(L, i + 1);
						}
					}

					handle_t h = core_managers_t::get()->texture_set_manager_->load(count, rids);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_texture_set_handle;
					return 1;
				}

				int32 l_load_material(lua_State *L)
				{
					const char *path = luaL_checkstring(L, 1);
					rid_t rid = engine_t::get()->assets_->path_to_rid(path);
					if(rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find material for path: %s", path);
						return 2;
					}
					handle_t h = core_managers_t::get()->material_manager_->load(rid);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_material_handle;
					return 1;
				}

				int32 l_load_mesh(lua_State *L)
				{
					const char *path = luaL_checkstring(L, 1);
					rid_t rid = engine_t::get()->assets_->path_to_rid(path);
					if(rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find mesh for path: %s", path);
						return 2;
					}
					handle_t h = core_managers_t::get()->mesh_manager_->load(rid);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_mesh_handle;
					return 1;
				}

				int32 l_load_vertex_format(lua_State *L)
				{
					luaL_error(L, "Not implemented");
					return 0;
				}

				int32 l_load_shader_program(lua_State *L)
				{
					const char *path = luaL_checkstring(L, 1);
					rid_t rid = engine_t::get()->assets_->path_to_rid(path);
					if(rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find shader program for path: %s", path);
						return 2;
					}
					handle_t h = core_managers_t::get()->shader_program_manager_->load(rid);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_shader_program_handle;
					return 1;
				}

				int32 l_load_font(lua_State *L)
				{
					const char *shader_program_path = luaL_checkstring(L, 1);
					const char *font_path = luaL_checkstring(L, 2);
					uint32 font_size = (uint32)luaL_optinteger(L, 3, 12);

					rid_t shader_program_rid = engine_t::get()->assets_->path_to_rid(shader_program_path);
					if(shader_program_rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find shader program for path: %s", shader_program_path);
						return 2;
					}

					rid_t font_rid = engine_t::get()->assets_->path_to_rid(font_path);
					if(font_rid == RID_NONE) {
						lua_pushnil(L);
						lua_pushfstring(L, "Could not find font for path: %s", font_path);
						return 2;
					}

					handle_t h = core_managers_t::get()->font_manager_->load(shader_program_rid, font_rid, font_size);
					udata_handle_t *data = luaL_pushhandle(L);
					data->handle_ = h;
					data->free_proc_ = &free_font_handle;
					return 1;
				}

				int32 l_load_skeleton(lua_State *L)
				{
					luaL_error(L, "Not implemented");
					return 0;
				}

				int32 l_load_skeleton_animation(lua_State *L)
				{
					luaL_error(L, "Not implemented");
					return 0;
				}

				static const luaL_Reg module[] = {
					{ "load_texture", l_load_texture },
					{ "load_texture_set", l_load_texture_set },
					{ "load_material", l_load_material },
					{ "load_mesh", l_load_mesh },
					{ "load_vertex_format", l_load_vertex_format },
					{ "load_shader_program", l_load_shader_program },
					{ "load_font", l_load_font },
					{ "load_skeleton", l_load_skeleton },
					{ "load_skeleton_animation", l_load_skeleton_animation },
					{ nullptr, nullptr }
				};

				LUALIB_API int32 luaopen_maki_assets(lua_State *L)
				{
					luaL_register(L, "maki.assets", module);
					return 0;
				}
			}


			const luaL_Reg modules[] = {
				{ "maki", maki::luaopen_maki },
				{ "maki.assets", assets::luaopen_maki_assets },
				{ "maki.window", window::luaopen_maki_window },
				{ "maki.renderer", renderer::luaopen_maki_renderer },
				{ nullptr, nullptr }
			};


		} // namespace lua

	} // namespace framework

} // namespace maki
