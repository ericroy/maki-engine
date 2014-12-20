#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTexture.h"
#include "core/MakiManager.h"

namespace maki
{
	namespace core
	{
		class texture_manager_t;
		

		class texture_manager_t : public manager_t<texture_t, texture_manager_t>
		{
		public:
			static const int32 default_size_ = 64;

		public:
			texture_manager_t(uint32 size = default_size_);	
			virtual ~texture_manager_t();
			handle_t load(rid_t rid);
			handle_t alloc_texture(texture_t::texture_type_t type, uint32 width, uint32 height, uint8 channels);
			void reload_assets();
			bool reload_asset(rid_t rid);

		private:
			bool load_data(texture_t *tex, rid_t rid);
		};

	} // namespace core

} // namespace maki
