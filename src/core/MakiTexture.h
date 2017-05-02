#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		enum texture_type_t {
			texture_type_regular = 0,
			texture_type_depth_stencil,
			texture_type_render_target,
			texture_type_max = texture_type_render_target
		};

		class texture_t {
			MAKI_NO_COPY(texture_t);
		public:
			texture_t() = default;
			~texture_t();
			bool init(texture_type_t type, uint32_t width, uint32_t height, uint8_t channels);
			bool load(rid_t rid);
		
			inline rid_t rid() const { return rid_; }
			inline void set_rid(rid_t rid) { rid_ = rid; }
			inline texture_type_t type() const { return type_; }
			inline uint32_t width() const { return width_; }
			inline uint32_t height() const { return height_; }
			inline uint32_t handle() const { return handle_; }
			inline void set_handle(uint32_t handle) { handle_ = handle; }

		private:
			rid_t rid_ = RID_NONE;
			texture_type_t type_ = texture_type_regular;
			uint32_t width_ = 0;
			uint32_t height_ = 0;
			uint32_t handle_ = (uint32_t)-1;
		};

	} // namespace core
} // namespace maki
