#pragma once
#include "core/MakiTypes.h"
#include "core/MakiMaterial.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class material_manager_t {
		public:
			material_manager_t(uint32_t capacity);
			virtual ~material_manager_t() = default;
			ref_t<material_t> create();
			ref_t<material_t> get(rid_t rid);
			ref_t<material_t> load(rid_t rid);
			ref_t<material_t> get_or_load(rid_t rid);
			ref_t<material_t> clone_if_shared(const ref_t<material_t> &mat);
		private:
			resource_pool_t<material_t> res_pool_;
		};

	} // namespace core
} // namespace maki
