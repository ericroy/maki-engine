#pragma once
#include "core/core_stdafx.h"
#include "core/MakiMaterial.h"
#include "core/MakiManager.h"

namespace maki
{
	namespace core
	{
		class material_manager_t;
		

		class material_manager_t : public manager_t<material_t, material_manager_t>
		{
		public:
			static handle_t duplicate_if_shared(handle_t handle);

		public:
			static const int32 default_size_ = 64;

		public:
			material_manager_t(uint32 size = default_size_);	
			virtual ~material_manager_t();	
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);

		private:
			void reload(material_t *mat);
		};

	} // namespace core

} // namespace maki
