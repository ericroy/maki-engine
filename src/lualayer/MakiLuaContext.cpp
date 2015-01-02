#include "lualayer/lualayer_stdafx.h"
#include "lualayer/MakiLuaContext.h"
#include "core/MakiConsole.h"
#include "core/MakiEngine.h"
#include "lualayer/modules/MakiModMaki.h"

using namespace maki::core;

namespace maki
{
	namespace lualayer
	{

		static const luaL_Reg builtins[] = {
			{ "maki", modules::luaopen_maki },
			{ nullptr, nullptr }
		};

		lua_context_t::lua_context_t()
			: L_(nullptr),
			update_function_ref_(LUA_REFNIL),
			draw_function_ref_(LUA_REFNIL)
		{
			L_ = lua_open();
			if(L_ == nullptr) {
				console_t::error("Lua init failed");
				return;
			}

			// Open standard libraries
			luaL_openlibs(L_);

			// Preload builtin modules
			lua_getglobal(L_, "package");
			lua_getfield(L_, -1, "preload");
			int32 index = 0;
			const luaL_Reg *reg = &builtins[index];
			while(reg->name != nullptr) {
				lua_pushcfunction(L_, reg->func);
				lua_setfield(L_, -2, reg->name);
				reg = &builtins[++index];
			}
			lua_pop(L_, 2);
		}
		
		lua_context_t::~lua_context_t()
		{
			if(L_ != nullptr) {
				lua_close(L_);
				L_ = nullptr;
			}
		}

		bool lua_context_t::eval(rid_t main_script)
		{
			bool ok = false;
			if(L_ == nullptr) {
				return ok;
			}

			uint32 bytes_read;
			auto eng = engine_t::get();
			char *data = eng->assets_->alloc_read(main_script, &bytes_read);
			if(data == nullptr) {
				console_t::lua_error("Failed to load lua script with Rid<%u>", main_script);
				goto cleanup;
			}

			int32 ret = luaL_loadbuffer(L_, data, bytes_read, eng->assets_->get_path(main_script));
			if(ret != 0) {
				console_t::lua_error(luaL_checklstring(L_, -1, nullptr));
				goto cleanup;
			}

			ret = lua_pcall(L_, 0, 0, 0);
			if(ret != 0) {
				console_t::lua_error(luaL_checklstring(L_, -1, nullptr));
				goto cleanup;
			}

			// Now that the main script is evaluated, it should have attached 'draw' and 'update'
			// functions to the maki module
			lua_getglobal(L_, "package");
			assert(lua_istable(L_, -1) && "Couldn't find package global");
			lua_getfield(L_, -1, "preload");
			assert(lua_istable(L_, -1) && "Couldn't find preload table");
			lua_getfield(L_, -1, "maki");
			if(lua_istable(L_, -1)) {
				lua_getfield(L_, -1, "update");
				if(lua_isfunction(L_, -1)) {
					update_function_ref_ = luaL_ref(L_, LUA_REGISTRYINDEX);
				} else {
					lua_pop(L_, 1);
				}

				lua_getfield(L_, -1, "draw");
				if(lua_isfunction(L_, -1)) {
					draw_function_ref_ = luaL_ref(L_, LUA_REGISTRYINDEX);
				} else {
					lua_pop(L_, 1);
				}
			} else {
				console_t::error("Failed to find preloaded table named 'maki'");
			}

			lua_pop(L_, 3);

			ok = true;

		cleanup:
			MAKI_SAFE_FREE(data);
			return ok;
		}

		void lua_context_t::update(float dt)
		{
			if(L_ == nullptr) {
				return;
			}

			if(update_function_ref_ == LUA_REFNIL) {
				return;
			}

			lua_getref(L_, update_function_ref_);
			if(lua_isfunction(L_, -1)) {
				lua_pushnumber(L_, dt);
				int32 ret = lua_pcall(L_, 1, 0, 0);
				if(ret != 0) {
					console_t::lua_error(luaL_checklstring(L_, -1, nullptr));
					lua_pop(L_, 1);
				}
			} else {
				lua_pop(L_, 1);
			}
		}

		void lua_context_t::draw()
		{
			if(L_ == nullptr) {
				return;
			}

			if(draw_function_ref_ == LUA_REFNIL) {
				return;
			}

			lua_getref(L_, draw_function_ref_);
			if(lua_isfunction(L_, -1)) {
				int32 ret = lua_pcall(L_, 0, 0, 0);
				if(ret != 0) {
					console_t::lua_error(luaL_checklstring(L_, -1, nullptr));
					lua_pop(L_, 1);
				}
			} else {
				lua_pop(L_, 1);
			}
		}

	} // namespace lualayer

} // namespace maki
