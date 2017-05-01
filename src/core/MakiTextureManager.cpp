#include "core/MakiTextureManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiConsole.h"

namespace maki {
	namespace core {

		texture_manager_t::texture_manager_t(uint32_t capacity) {
			res_pool_.reset(new resource_pool_t<texture_t>(capacity));
		}

		ref_t<texture_t> texture_manager_t::get(rid_t rid) {
			return res_pool_->find([rid](const texture_t &res) { return rid == res.rid(); });
		}

		ref_t<texture_t> texture_manager_t::load(rid_t rid) {
			auto *eng = engine_t::get();
			auto data = eng->assets->alloc_read(rid);
			if (!data)
				return nullptr;
			auto tex = res_pool_->alloc();
			if (!eng->renderer->create_texture(tex, data.data(), data.length())) {
				console_t::error("Failed to create texture <rid %ull>", rid);
				return nullptr;
			}
			tex->set_rid(rid);
			return tex;
		}

		ref_t<texture_t> texture_manager_t::get_or_load(rid_t rid) {
			auto ref = get(rid);
			return ref ? ref : load(rid);
		}
	
		ref_t<texture_t> texture_manager_t::create(texture_t::texture_type_t type, uint32_t width, uint32_t height, uint8_t channels) {
			auto *eng = engine_t::get();

			auto tex = (*res_pool_)->alloc();
			tex->type_ = type;
			tex->width_ = width;
			tex->height_ = height;

			switch(tex->type_) {
			case texture_t::texture_type_regular:
				if(eng->renderer->create_empty_texture(tex.ptr(), channels))
					return tex;
				break;
			case texture_t::texture_type_depth_stencil:
				if(eng->renderer->create_depth_texture(tex.ptr()))
					return tex;
				break;
			case texture_t::texture_type_render_target:
				if(eng->renderer->create_render_target(tex.ptr()))
					return tex;
				break;
			default:
				MAKI_ASSERT(false);
				break;
			}

			console_t::error("Failed to alloc empty texture");
			return nullptr;
		}


	} // namespace core
} // namespace maki
