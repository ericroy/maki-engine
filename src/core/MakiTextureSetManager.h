#pragma once
#include "core/MakiTypes.h"
#include "core/MakiTextureSet.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class texture_set_manager_t {
			MAKI_NO_COPY(texture_set_manager_t);
		public:
			texture_set_manager_t(uint32_t capacity);
			virtual ~texture_set_manager_t() = default;
			ref_t<texture_set_t> create();
			ref_t<texture_set_t> get(uint8_t count, rid_t *rids);
			ref_t<texture_set_t> load(uint8_t count, rid_t *rids);
			ref_t<texture_set_t> get_or_load(uint8_t count, rid_t *rids);
		private:
			unique_ptr<resouce_pool_t<texture_set_t>> res_pool_;
		};


	} // namespace core
} // namespace maki
