#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiMesh.h"

namespace maki
{
	namespace core
	{
		class mesh_manager_t;
		

		class mesh_manager_t : public manager_t<mesh_t, mesh_manager_t>
		{
		public:
			static const int32 default_size_ = 64;

		public:
			mesh_manager_t(uint32 size = default_size_);	
			virtual ~mesh_manager_t();	
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core

} // namespace maki
