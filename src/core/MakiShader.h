#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiDocument.h"

namespace maki
{
	namespace core
	{

		class shader_t
		{
		public:
			static const uint32_t uniform_name_max_length_ = 32;

			struct material_uniform_location_t
			{
			public:
				material_uniform_location_t();
				material_uniform_location_t(int32_t location, char *name);

			public:
				int32_t location_;
				char name_[uniform_name_max_length_];
			};
					
			enum frame_uniform_t
			{
				frame_uniform_none_ = -1,
				frame_uniform_view_ = 0,
				frame_uniform_projection_,
				frame_uniform_view_projection_,
				frame_uniform_light_positions_,
				frame_uniform_light_directions_,
				frame_uniform_light_view_proj_,
				frame_uniform_light_properties_,
				frame_uniform_shadow_map_properties_,
				frame_uniform_light_split_regions_,
				frame_uniform_camera_split_distances_,
				frame_uniform_global_ambient_color_,
				frame_uniform_camera_with_height_hear_far_,
				frame_uniform_count_
			};

			enum object_uniform_t
			{
				object_uniform_none_ = -1,
				object_uniform_model_ = 0,
				object_uniform_model_view_,
				object_uniform_model_view_projection_,
				object_uniform_count_
			};
		
			static frame_uniform_t get_frame_uniform_by_name(const char *name);
			static object_uniform_t get_object_uniform_by_name(const char *name);

			static const char *frame_uniform_names_[frame_uniform_count_];
			static const char *object_uniform_names_[object_uniform_count_];

		public:
			shader_t();
			~shader_t();

			bool init(document_t::node_t *shader_node, const char *data_key, const char *meta_key);
			int32_t find_material_constant_location(const char *name);

		private:
			shader_t(const shader_t &other) {}

		public:
			int32_t frame_uniform_buffer_location_;
			uint32_t engine_frame_uniform_bytes_;
			int32_t engine_frame_uniform_locations_[frame_uniform_count_];

			int32_t object_uniform_buffer_location_;
			uint32_t engine_object_uniform_bytes_;
			int32_t engine_object_uniform_locations_[object_uniform_count_];

			int32_t material_uniform_buffer_location_;
			uint32_t material_uniform_bytes_;

			std::vector<material_uniform_location_t> material_uniform_locations_;

			
			char *program_data_;
			uint32_t program_data_bytes_;

			intptr_t handle_;
		};

		

	} // namespace core

} // namespace maki
