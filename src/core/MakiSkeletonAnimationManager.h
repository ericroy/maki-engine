#pragma once
#include "core/MakiTypes.h"
#include "core/MakiSkeletonAnimation.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class skeleton_animation_manager_t {
			MAKI_NO_COPY(skeleton_animation_manager_t);
		public:
			skeleton_animation_manager_t(uint32_t capacity);
			virtual ~skeleton_animation_manager_t() = default;
			ref_t<skeleton_animation_t> get(rid_t rid);
			ref_t<skeleton_animation_t> load(rid_t rid);
			ref_t<skeleton_animation_t> get_or_load(rid_t rid);
		private:
			resource_pool_t<skeleton_animation_t> res_pool_;
		};

	} // namespace core
} // namespace maki
