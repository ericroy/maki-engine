#pragma once
#include "core/MakiTypes.h"
#include "core/MakiArray.h"
#include "core/MakiTexture.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class texture_manager_t {
			MAKI_NO_COPY(texture_manager_t);
		public:
			texture_manager_t(uint32_t capacity);
			virtual ~texture_manager_t() = default;
			ref_t<texture_t> get(rid_t rid);
			ref_t<texture_t> load(rid_t rid);
			ref_t<texture_t> get_or_load(rid_t rid);
			ref_t<texture_t> create(texture_t::texture_type_t type, uint32_t width, uint32_t height, uint8_t channels);
		private:
			unique_ptr<resource_pool_t<texture_t>> res_pool_;
		};

	} // namespace core
} // namespace maki
