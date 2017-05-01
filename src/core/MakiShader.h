#pragma once
#include "core/MakiMacros.h"
#include "core/MakiResource.h"
#include "core/MakiDocument.h"

namespace maki {
	namespace core {

		class shader_t {
			MAKI_NO_COPY(shader_t);

		public:
			static const uint32_t uniform_name_max_length = 32;

			struct material_uniform_location_t {
			public:
				material_uniform_location_t() = default;
				material_uniform_location_t(int32_t location, char *name);

			public:
				int32_t location = -1;
				char name[uniform_name_max_length] = "";
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
				frame_uniform_count
			};

			enum object_uniform_t {
				object_uniform_none = -1,
				object_uniform_model = 0,
				object_uniform_model_view,
				object_uniform_model_view_projection,
				object_uniform_count
			};
		
			static frame_uniform_t get_frame_uniform_by_name(const char *name);
			static object_uniform_t get_object_uniform_by_name(const char *name);

			static const char *frame_uniform_names[frame_uniform_count];
			static const char *object_uniform_names[object_uniform_count];

		public:
			shader_t() = default;
			~shader_t();

			bool init(const document_t::node_t &shader_node, const char *data_key, const char *meta_key);
			int32_t find_material_constant_location(const char *name);

		public:
			int32_t frame_uniform_buffer_location = -1;
			uint32_t engine_frame_uniform_bytes = 0;
			int32_t engine_frame_uniform_locations[frame_uniform_count] = {};

			int32_t object_uniform_buffer_location = -1;
			uint32_t engine_object_uniform_bytes = 0;
			int32_t engine_object_uniform_locations[object_uniform_count] = {};

			int32_t material_uniform_buffer_location = -1;
			uint32_t material_uniform_bytes = 0;

			std::vector<material_uniform_location_t> material_uniform_locations;

			char *program_data = nullptr;
			uint32_t program_data_bytes = 0;

			intptr_t handle = 0;
		};

		

	} // namespace core
} // namespace maki
