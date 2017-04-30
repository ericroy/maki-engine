#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiSkeletonAnimation.h"

namespace maki
{
	namespace core
	{

		class skeleton_animation_manager_t;
		

		class skeleton_animation_manager_t : public manager_t<skeleton_animation_t, skeleton_animation_manager_t>
		{
		public:
			static const int32_t default_size_ = 32;

		public:
			skeleton_animation_manager_t(uint32_t size = default_size_);	
			virtual ~skeleton_animation_manager_t();
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core

} // namespace maki
