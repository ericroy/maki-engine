#pragma once
#include "core/MakiMacros.h"
#include "core/MakiManager.h"
#include "core/MakiMesh.h"

namespace maki {
	namespace core {

		class mesh_manager_t : public manager_t<mesh_t, mesh_manager_t> {
			MAKI_NO_COPY(mesh_manager_t);

		public:
			mesh_manager_t(uint64_t capacity);	
			virtual ~mesh_manager_t() = default;
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core
} // namespace maki
