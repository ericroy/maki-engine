#pragma once
#include "core/MakiTypes.h"
#include "core/MakiDocument.h"
#include "core/MakiArray.h"

namespace maki {
	namespace core {

		enum frame_constant_t {
			frame_constant_none = -1,
			frame_constant_view = 0,
			frame_constant_projection,
			frame_constant_view_projection,
			frame_constant_light_positions,
			frame_constant_light_directions,
			frame_constant_light_view_proj,
			frame_constant_light_properties,
			frame_constant_shadow_map_properties,
			frame_constant_light_split_regions,
			frame_constant_camera_split_distances,
			frame_constant_global_ambient_color,
			frame_constant_camera_with_height_hear_far,
			frame_constant_max = frame_constant_camera_with_height_hear_far,
		};

		enum object_constant_t {
			object_constant_none = -1,
			object_constant_model = 0,
			object_constant_model_view,
			object_constant_model_view_projection,
			object_constant_max = object_constant_model_view_projection
		};

		frame_constant_t get_frame_constant_by_name(const char *name);
		object_constant_t get_object_constant_by_name(const char *name);


		class shader_t {
			MAKI_NO_COPY(shader_t);

			struct material_constant_t {
			public:
				material_constant_t() = default;
				material_constant_t(int32_t location, const char *name);
			public:
				int32_t location = -1;
				char name[32] = "";
			};

		public:
			shader_t() = default;

			inline int32_t frame_constant_buffer_location() const { return frame_constant_buffer_location_; }
			inline uint32_t engine_frame_constant_bytes() const { return engine_frame_constant_bytes_; }
			inline int32_t get_engine_frame_constant_location(frame_constant_t uni) const { return engine_frame_constant_locations_[uni]; }

			inline int32_t object_constant_buffer_location() const { return object_constant_buffer_location_; }
			inline uint32_t engine_object_constant_bytes() const { return engine_object_constant_bytes_; }
			inline int32_t get_engine_object_constant_location(object_constant_t uni) const { return engine_object_constant_locations_[uni]; }

			inline int32_t material_constant_buffer_location() const { return material_constant_buffer_location_; }
			inline uint32_t material_constant_bytes() const { return material_constant_bytes_; }

			inline void *handle() const { return handle_; }
			inline void set_handle(void *handle) { handle_ = handle; }

			inline const array_t<char> &program_data() const { return program_data_; }

			bool init(const document_t::node_t &shader_node, const char *data_key, const char *meta_key);
			int32_t get_material_constant_location(const char *name) const;

		private:
			int32_t frame_constant_buffer_location_ = -1;
			uint32_t engine_frame_constant_bytes_ = 0;
			int32_t engine_frame_constant_locations_[frame_constant_max + 1] = {};

			int32_t object_constant_buffer_location_ = -1;
			uint32_t engine_object_constant_bytes_ = 0;
			int32_t engine_object_constant_locations_[object_constant_max + 1] = {};

			int32_t material_constant_buffer_location_ = -1;
			uint32_t material_constant_bytes_ = 0;
			vector<material_constant_t> material_constant_locations_;

			array_t<char> program_data_;
			void *handle_ = 0;
		};

		

	} // namespace core
} // namespace maki
