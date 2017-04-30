#include "framework/framework_stdafx.h"
#include "framework/lua/MakiLua_maki_assets.h"
#include "framework/lua/MakiLua_maki.h"

using namespace maki::core;

namespace maki
{
	namespace framework
	{
		namespace lua
		{
			
			const char *udata_handle_t::type_name_ = "maki.assets.Handle";

			int32_t udata_handle_t::gc(lua_State *L)
			{
				udata_handle_t *data = luaL_toudata<udata_handle_t>(L, 1);
				data->free_proc_(data->handle_);
				return 0;
			}

			int32_t udata_handle_t::tostring(lua_State *L)
			{
				lua_pushfstring(L, "<%s: %p>", udata_handle_t::type_name_, lua_touserdata(L, 1));
				return 1;
			}

			const luaL_reg udata_handle_t::methods_[] = {
				{ "free", udata_handle_t::gc },
				{ nullptr, nullptr }
			};

			const luaL_reg udata_handle_t::metamethods_[] = {
				{ "__gc", udata_handle_t::gc },
				{ "__tostring", udata_handle_t::tostring },
				{ nullptr, nullptr }
			};



			static void free_texture_handle(handle_t &h) { manager_t<mesh_t, mesh_manager_t>::free(h); }
			static void free_texture_set_handle(handle_t &h) { manager_t<texture_set_t, texture_set_manager_t>::free(h); }
			static void free_material_handle(handle_t &h) { manager_t<material_t, material_manager_t>::free(h); }
			static void free_mesh_handle(handle_t &h) { manager_t<mesh_t, mesh_manager_t>::free(h); }
			static void free_vertex_format_handle(handle_t &h) { manager_t<vertex_format_t, vertex_format_manager_t>::free(h); }
			static void free_shader_program_handle(handle_t &h) { manager_t<shader_program_t, shader_program_manager_t>::free(h); }
			static void free_font_handle(handle_t &h) { manager_t<font_t, font_manager_t>::free(h); }
			static void free_skeleton_handle(handle_t &h) { manager_t<skeleton_t, skeleton_manager_t>::free(h); }
			static void free_skeleton_animation_handle(handle_t &h) { manager_t<skeleton_animation_t, skeleton_animation_manager_t>::free(h); }

			int32_t l_load_texture(lua_State *L)
			{
				const char *path = luaL_checkstring(L, 1);
				rid_t rid = engine_t::get()->assets_->path_to_rid(path);
				if(rid == RID_NONE) {
					lua_pushnil(L);
					lua_pushfstring(L, "Could not find texture for path: %s", path);
					return 2;
				}
				handle_t h = core_managers_t::get()->texture_manager_->load(rid);
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_texture_handle;
				return 1;
			}

			int32_t l_load_texture_set(lua_State *L)
			{
				rid_t rids[texture_set_t::max_textures_per_set_];
				int32_t count = lua_gettop(L);
				if(lua_type(L, 1) == LUA_TSTRING) {
					for(int32_t i = 0; i < count; i++) {
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
					for(int32_t i = 0; i < count; i++) {
						const char *path = luaL_checkstring(L, i + 1);
						rids[i] = luaL_checkinteger(L, i + 1);
					}
				}

				handle_t h = core_managers_t::get()->texture_set_manager_->load(count, rids);
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_texture_set_handle;
				return 1;
			}

			int32_t l_load_material(lua_State *L)
			{
				const char *path = luaL_checkstring(L, 1);
				rid_t rid = engine_t::get()->assets_->path_to_rid(path);
				if(rid == RID_NONE) {
					lua_pushnil(L);
					lua_pushfstring(L, "Could not find material for path: %s", path);
					return 2;
				}
				handle_t h = core_managers_t::get()->material_manager_->load(rid);
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_material_handle;
				return 1;
			}

			int32_t l_load_mesh(lua_State *L)
			{
				const char *path = luaL_checkstring(L, 1);
				rid_t rid = engine_t::get()->assets_->path_to_rid(path);
				if(rid == RID_NONE) {
					lua_pushnil(L);
					lua_pushfstring(L, "Could not find mesh for path: %s", path);
					return 2;
				}
				handle_t h = core_managers_t::get()->mesh_manager_->load(rid);
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_mesh_handle;
				return 1;
			}

			int32_t l_load_vertex_format(lua_State *L)
			{
				luaL_error(L, "Not implemented");
				return 0;
			}

			int32_t l_load_shader_program(lua_State *L)
			{
				const char *path = luaL_checkstring(L, 1);
				rid_t rid = engine_t::get()->assets_->path_to_rid(path);
				if(rid == RID_NONE) {
					lua_pushnil(L);
					lua_pushfstring(L, "Could not find shader program for path: %s", path);
					return 2;
				}
				handle_t h = core_managers_t::get()->shader_program_manager_->load(rid);
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_shader_program_handle;
				return 1;
			}

			int32_t l_load_font(lua_State *L)
			{
				const char *shader_program_path = luaL_checkstring(L, 1);
				const char *font_path = luaL_checkstring(L, 2);
				uint32_t font_size = (uint32_t)luaL_optinteger(L, 3, 12);

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
				udata_handle_t *data = luaL_pushudata<udata_handle_t>(L);
				data->handle_ = h;
				data->free_proc_ = &free_font_handle;
				return 1;
			}

			int32_t l_load_skeleton(lua_State *L)
			{
				luaL_error(L, "Not implemented");
				return 0;
			}

			int32_t l_load_skeleton_animation(lua_State *L)
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

			LUALIB_API int32_t luaopen_maki_assets(lua_State *L)
			{
				luaL_register(L, "maki.assets", module);

				lua_pushliteral(L, "Handle");
				register_type<udata_handle_t>(L);
				lua_rawset(L, -3);

				return 0;
			}


		} // namespace lua
	} // namespace framework
} // namespace maki
