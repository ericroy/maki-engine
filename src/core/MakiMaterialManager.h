#pragma once
#include "core/MakiMaterial.h"
#include "core/MakiManager.h"

namespace maki {
	namespace core {

		class material_manager_t : public manager_t<material_t, material_manager_t> {
		public:
			static handle_t duplicate_if_shared(handle_t handle);

		public:
			material_manager_t(uint64_t capacity);	
			virtual ~material_manager_t() = default;
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);

		private:
			void reload(material_t *mat);
		};

	} // namespace core
} // namespace maki
