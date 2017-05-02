#include "core/MakiTexture.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace maki {
	namespace core {

		texture_t::~texture_t() {
			engine_t::get()->renderer->delete_texture(this);
		}

		bool texture_t::init(texture_type_t type, uint32_t width, uint32_t height, uint8_t channels) {
			MAKI_ASSERT(handle_ == (uint32_t)-1 && "Texture already initialized");
			type_ = type;
			width_ = width;
			height_ = height;

			auto *eng = engine_t::get();
			switch(type_) {
			case texture_t::texture_type_regular:
				if(eng->renderer->create_empty_texture(this, channels))
					return true;
				break;
			case texture_t::texture_type_depth_stencil:
				if(eng->renderer->create_depth_texture(this))
					return true;
				break;
			case texture_t::texture_type_render_target:
				if(eng->renderer->create_render_target(this))
					return true;
				break;
			default:
				MAKI_ASSERT(false);
				break;
			}

			console_t::error("Failed to alloc empty texture");
			return nullptr;
		}
		
		bool texture_t::load(rid_t rid) {
			MAKI_ASSERT(handle_ == (uint32_t)-1 && "Texture already initialized");
			auto *eng = engine_t::get();
			auto data = eng->assets->alloc_read(rid);
			if (!data)
				return false;
			if (!eng->renderer->create_texture(this, data.data(), data.length())) {
				console_t::error("Failed to create texture <rid %u>", rid);
				return false;
			}
			rid_ = rid;
			return tex;
		}
	
	} // namespace core
} // namespace maki
