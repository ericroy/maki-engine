#pragma once
#include "core/MakiTypes.h"
#include "core/MakiDocument.h"
#include "core/MakiArray.h"

namespace maki {
	namespace core {

		struct material_uniform_location_t {
		public:
			material_uniform_location_t() = default;
			material_uniform_location_t(int32_t location, char *name);

		public:
			int32_t location = -1;
			char name[32] = "";
		};

		enum frame_uniform_t {
			frame_uniform_none = -1,
			frame_uniform_view = 0,
			frame_uniform_projection,
			frame_uniform_view_projection,
			frame_uniform_light_positions,
			frame_uniform_light_directions,
			frame_uniform_light_view_proj,
			frame_uniform_light_properties,
			frame_uniform_shadow_map_properties,
			frame_uniform_light_split_regions,
			frame_uniform_camera_split_distances,
			frame_uniform_global_ambient_color,
			frame_uniform_camera_with_height_hear_far,
			frame_uniform_max = frame_uniform_camera_with_height_hear_far,
		};

		enum object_uniform_t {
			object_uniform_none = -1,
			object_uniform_model = 0,
			object_uniform_model_view,
			object_uniform_model_view_projection,
			object_uniform_max = object_uniform_model_view_projection
		};

		frame_uniform_t get_frame_uniform_by_name(const char *name);
		object_uniform_t get_object_uniform_by_name(const char *name);


		class shader_t {
			MAKI_NO_COPY(shader_t);

		public:
			shader_t() = default;

			inline int32_t frame_uniform_buffer_location() const { return frame_uniform_buffer_location_; }
			inline uint32_t engine_frame_uniform_bytes() const { return engine_frame_uniform_bytes_; }
			inline int32_t get_engine_frame_uniform_location(frame_uniform_t uni) const { return engine_frame_uniform_locations_[uni]; }

			inline int32_t object_uniform_buffer_location() const { return object_uniform_buffer_location_; }
			inline uint32_t engine_object_uniform_bytes() const { return engine_object_uniform_bytes_; }
			inline int32_t get_engine_object_uniform_location(object_uniform_t uni) const { return engine_object_uniform_locations_[uni]; }

			inline int32_t material_uniform_buffer_location() const { return material_uniform_buffer_location_; }
			inline uint32_t material_uniform_bytes() const { return material_uniform_bytes_; }

			inline uintptr_t handle() const { return handle_; }
			inline void set_handle(uintptr_t handle) { handle_ = handle; }


			bool init(const document_t::node_t &shader_node, const char *data_key, const char *meta_key);
			int32_t find_material_constant_location(const char *name);

		private:
			int32_t frame_uniform_buffer_location_ = -1;
			uint32_t engine_frame_uniform_bytes_ = 0;
			int32_t engine_frame_uniform_locations_[frame_uniform_max + 1] = {};

			int32_t object_uniform_buffer_location_ = -1;
			uint32_t engine_object_uniform_bytes_ = 0;
			int32_t engine_object_uniform_locations_[object_uniform_max + 1] = {};

			int32_t material_uniform_buffer_location_ = -1;
			uint32_t material_uniform_bytes_ = 0;
			vector<material_uniform_location_t> material_uniform_locations_;

			array_t<char> program_data_;
			uintptr_t handle_ = 0;
		};

		

	} // namespace core
} // namespace maki
