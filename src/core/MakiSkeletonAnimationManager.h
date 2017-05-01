#pragma once
#include "core/MakiMacros.h"
#include "core/MakiManager.h"
#include "core/MakiSkeletonAnimation.h"

namespace maki {
	namespace core {

		class skeleton_animation_manager_t : public manager_t<skeleton_animation_t, skeleton_animation_manager_t> {
		public:
			skeleton_animation_manager_t(uint64_t capacity);
			virtual ~skeleton_animation_manager_t() = default;
			handle_t load(rid_t rid);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core
} // namespace maki
