#include "core/MakiTextureManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"

namespace maki {
	namespace core {

		texture_manager_t::texture_manager_t(uint64_t capacity)
			: manager_t<texture_t, texture_manager_t>(capacity, "texture_manager_t") {
		}
	
		handle_t texture_manager_t::alloc_texture(texture_t::texture_type_t type, uint32_t width, uint32_t height, uint8_t channels) {
			engine_t *eng = engine_t::get();

			handle_t handle_ = res_pool_->alloc() | manager_id_;
			texture_t *tex_ = res_pool_->get(handle_ & handle_value_mask_);
			new(tex_) texture_t();
			tex_->type_ = type;
			tex_->width_ = width;
			tex_->height_ = height;

			switch(tex_->type_) {
			case texture_t::texture_type_regular_:
				if(eng->renderer_->create_empty_texture(tex_, channels))
					return handle_;
				break;
			case texture_t::texture_type_depth_stencil_:
				if(eng->renderer_->create_depth_texture(tex_))
					return handle_;
				break;
			case texture_t::texture_type_render_target_:
				if(eng->renderer_->create_render_target(tex_))
					return handle_;
				break;
			default:
				assert(false);
				break;
			}

			console_t::error("Failed to alloc empty texture");
			res_pool_->free(handle_ & handle_value_mask_);
			return HANDLE_NONE;
		}

		handle_t texture_manager_t::load(rid_t rid) {
			handle_t handle_ = res_pool_->match(resource_t::find_predicate_t<texture_t>(rid)) | manager_id_;
			if(handle_ != HANDLE_NONE)
				return handle_;

			handle_ = res_pool_->alloc() | manager_id_;
			texture_t *tex_ = res_pool_->get(handle_ & handle_value_mask_);
			new(tex_) texture_t();
		
			if(!load_data(tex_, rid)) {
				res_pool_->free(handle_ & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle_;
		}

		bool texture_manager_t::load_data(texture_t *tex_, rid_t rid) {
			engine_t *eng = engine_t::get();

			uint32_t bytes_read;
			char *data = eng->assets_->alloc_read(rid, &bytes_read);

			if(!eng->renderer_->create_texture(tex_, (char *)data, bytes_read)) {
				console_t::error("Failed to create texture <rid %ull>", rid);
				MAKI_SAFE_FREE(data);
				return false;
			}

			MAKI_SAFE_FREE(data);
			tex_->rid_ = rid;
			return true;
		}

		void texture_manager_t::reload_assets() {
			const resource_pool_t<texture_t>::iterator_t end = res_pool_->end();
			for(resource_pool_t<texture_t>::iterator_t iter = res_pool_->begin(); iter != end; ++iter) {
				texture_t *tex_ = iter.ptr();
				rid_t rid = tex_->rid_;
				if(rid != RID_NONE) {
					tex_->~texture_t();
					new(tex_) texture_t();
					load_data(tex_, rid);
				}
			}
		}

		bool texture_manager_t::reload_asset(rid_t rid) {
			handle_t handle_ = res_pool_->match(resource_t::find_predicate_t<texture_t>(rid)) | manager_id_;
			if(handle_ == HANDLE_NONE)
				return false;

			texture_t *tex_ = res_pool_->get(handle_ & handle_value_mask_);
			res_pool_->free(handle_ & handle_value_mask_);

			if(rid != RID_NONE) {
				tex_->~texture_t();
				new(tex_) texture_t();
				load_data(tex_, rid);
			}
			return true;
		}


	} // namespace core
} // namespace maki
