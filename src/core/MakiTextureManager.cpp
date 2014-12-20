#include "core/core_stdafx.h"
#include "core/MakiTextureManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"

namespace maki
{
	namespace core
	{

		texture_manager_t::texture_manager_t(uint32 size)
			: manager_t<texture_t, texture_manager_t>(size, "texture_manager_t")
		{
		}
	
		texture_manager_t::~texture_manager_t()
		{
		}
	
		handle_t texture_manager_t::alloc_texture(texture_t::texture_type_t type, uint32 width, uint32 height, uint8 channels)
		{
			engine_t *eng = engine_t::get();

			handle_t handle = res_pool_->alloc() | manager_id_;
			texture_t *tex = res_pool_->get(handle & handle_value_mask_);
			new(tex) texture_t();
			tex->type_ = type;
			tex->width_ = width;
			tex->height_ = height;

			switch(tex->type_) {
			case texture_t::texture_type_regular_:
				if(eng->renderer_->create_empty_texture(tex, channels)) {
					return handle;
				}
				break;
			case texture_t::texture_type_depth_stencil_:
				if(eng->renderer_->create_depth_texture(tex)) {
					return handle;
				}
				break;
			case texture_t::texture_type_render_target_:
				if(eng->renderer_->create_render_target(tex)) {
					return handle;
				}
				break;
			default:
				assert(false);
				break;
			}

			console_t::error("Failed to alloc empty texture");
			res_pool_->free(handle & handle_value_mask_);
			return HANDLE_NONE;
		}

		handle_t texture_manager_t::load(rid_t rid)
		{
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<texture_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = res_pool_->alloc() | manager_id_;
			texture_t *tex = res_pool_->get(handle & handle_value_mask_);
			new(tex) texture_t();
		
			if(!load_data(tex, rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		bool texture_manager_t::load_data(texture_t *tex, rid_t rid)
		{
			engine_t *eng = engine_t::get();

			uint32 bytesRead;
			char *data = eng->assets_->alloc_read(rid, &bytesRead);

			if(!eng->renderer_->create_texture(tex, (char *)data, bytesRead)) {
				console_t::error("Failed to create texture <rid %d>", rid);
				MAKI_SAFE_FREE(data);
				return false;
			}

			MAKI_SAFE_FREE(data);
			tex->rid_ = rid;
			return true;
		}

		void texture_manager_t::reload_assets()
		{
			const resource_pool_t<texture_t>::iterator_t end = res_pool_->end();
			for(resource_pool_t<texture_t>::iterator_t iter = res_pool_->begin(); iter != end; ++iter) {
				texture_t *tex = iter.Ptr();
				rid_t rid = tex->rid_;
				if(rid != RID_NONE) {
					tex->~texture_t();
					new(tex) texture_t();
					load_data(tex, rid);
				}
			}
		}

		bool texture_manager_t::reload_asset(rid_t rid)
		{
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<texture_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE) {
				return false;
			}
			texture_t *tex = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				tex->~texture_t();
				new(tex) texture_t();
				load_data(tex, rid);
			}
			return true;
		}


	} // namespace core

} // namespace maki
