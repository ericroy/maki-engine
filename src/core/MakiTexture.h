#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class texture_t {
			MAKI_NO_COPY(texture_t);

		public:
			enum texture_type_t {
				texture_type_regular = 0,
				texture_type_depth_stencil,
				texture_type_render_target,
			};

		public:
			texture_t() = default;
			~texture_t();

		public:
			rid_t rid = RID_NONE;
			texture_type_t type = texture_type_regular;
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t handle = (uint32_t)-1;
		};

	} // namespace core
} // namespace maki
