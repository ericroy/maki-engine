#pragma once
#include "core/MakiMacros.h"
#include "core/MakiTexture.h"
#include "core/MakiManager.h"

namespace maki {
	namespace core {

		class texture_manager_t : public manager_t<texture_t, texture_manager_t>
		{
			MAKI_NO_COPY(texture_manager_t);

		public:
			texture_manager_t(uint64_t capacity);
			virtual ~texture_manager_t() = default;
			handle_t load(rid_t rid);
			handle_t alloc_texture(texture_t::texture_type_t type, uint32_t width, uint32_t height, uint8_t channels);
			void reload_assets();
			bool reload_asset(rid_t rid);

		private:
			bool load_data(texture_t *tex_, rid_t rid);
		};

	} // namespace core
} // namespace maki
