#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiSkeleton.h"

namespace maki
{
	namespace core
	{

		class skeleton_manager_t;
		

		class skeleton_manager_t : public manager_t<skeleton_t, skeleton_manager_t>
		{
		public:
			static const int32_t default_size_ = 8;

		public:
			skeleton_manager_t(uint32_t size = default_size_);	
			virtual ~skeleton_manager_t();
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core

} // namespace maki
