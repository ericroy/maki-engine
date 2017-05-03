#pragma once
#include "core/MakiTypes.h"
#include "core/MakiMesh.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class mesh_manager_t {
			MAKI_NO_COPY(mesh_manager_t);
		public:
			mesh_manager_t(uint32_t capacity);
			virtual ~mesh_manager_t() = default;
			ref_t<mesh_t> create();
			ref_t<mesh_t> get(rid_t rid);
			ref_t<mesh_t> load(rid_t rid);
			ref_t<mesh_t> get_or_load(rid_t rid);
		private:
			resource_pool_t<mesh_t> res_pool_;
		};

	} // namespace core
} // namespace maki
