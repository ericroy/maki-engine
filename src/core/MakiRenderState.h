#pragma once
#include "core/core_stdafx.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiTextureManager.h"


namespace maki
{
	namespace core
	{

		class __declspec(align(MAKI_SIMD_ALIGN)) render_state_t : public aligned_t<MAKI_SIMD_ALIGN>
		{
		public:
			static const int32 max_lights_ = 8;
			static const int32 max_shadow_lights_ = 2;
			static const int32 MAX_SPLIT_SHADOW_LIGHTS = 2;
			static const int32 max_cascades_ = 5;

			enum render_target_t
			{
				render_target_null_ = 0,
				render_target_default_,
				render_target_custom_
			};
			enum depth_stencil_t
			{
				depth_stencil_null_ = 0,
				depth_stencil_default_,
				depth_stencil_custom_
			};
			enum cull_mode_t
			{
				cull_mode_none_ = 0,
				cull_mode_front_,
				cull_mode_back_
			};
			enum depth_test_t
			{
				depth_test_disabled_ = 0,
				depth_test_less_,
				depth_test_equal_,
				depth_test_less_equal_
			};
			enum light_type_t
			{
				light_type_directional_ = 0,
				light_type_point_,
				light_type_spot_,
				light_type_count_
			};

			enum light_flag_t
			{
				light_flag_type_directional_ = 1<<light_type_directional_,
				light_flag_type_point_ = 1<<light_type_point_,
				light_flag_type_spot_ = 1<<light_type_spot_,

				light_flag_on_ = 1<<3,
				light_flag_shadow_ = 1<<4,
				light_flag_split_shadow_ = 1<<5,
			};

			// Must match the light properties struct defined in shaders
			struct light_properties_t
			{
				vector4_t ambient_color_;
				vector4_t diffuse_color_;
				vector4_t specular_color_;
				vector4_t width_height_near_far_;
				float attenuation_;
				float spot_factor_;
				float fov_;
				uint32 flags_;
			};

			struct shadow_map_properties_t
			{
				vector2_t size_;
				float pad_[2];
			};

			struct camera_split_distances_t
			{
				float splits_[max_cascades_-1];
			};

			struct __declspec(align(MAKI_SIMD_ALIGN)) light_split_region_t : public aligned_t<MAKI_SIMD_ALIGN>
			{
				matrix44_t view_proj_;
				vector4_t width_height_near_far;
			};

		public:
			render_state_t()
			{
				memset(light_properties_, 0, sizeof(light_properties_));
				render_target_type_ = render_target_default_;
				depth_stencil_type_ = depth_stencil_default_;
				clear_render_target_ = false;
				clear_depth_stencil_ = false;
				depth_test_ = depth_test_less_;
				depth_write_ = true;
				wire_frame_ = false;
				global_ambient_color_ = vector4_t(0.0f);
				cull_mode_ = cull_mode_back_;
				shader_variant_ = shader_program_t::variant_normal_;
			
				light_count_ = 0;
				shadow_light_count_ = 0;
				cascaded_shadow_light_count_ = 0;

				render_target_ = HANDLE_NONE;
				depth_stencil_ = HANDLE_NONE;
				for(uint32 i = 0; i < max_shadow_lights_; i++) {
					shadow_maps_[i] = HANDLE_NONE;
				}
			}

			~render_state_t()
			{
				clear();
			}

			void copy(const render_state_t &s)
			{
				*this = s;
				for(uint32 i = 0; i < max_shadow_lights_; i++) {
					texture_manager_t::add_ref(shadow_maps_[i]);
				}
				texture_manager_t::add_ref(render_target_);
				texture_manager_t::add_ref(depth_stencil_);
			}

			void clear()
			{
				for(uint32 i = 0; i < max_shadow_lights_; i++) {
					texture_manager_t::free(shadow_maps_[i]);
				}
				texture_manager_t::free(render_target_);
				texture_manager_t::free(depth_stencil_);
			}

		private:
			render_state_t(const render_state_t &other) {}

		public:
			// Camera data
			matrix44_t view_;
			matrix44_t projection_;
			camera_split_distances_t camera_split_distances;
			vector4_t camera_width_height_near_far;

			// light_t and shadow data
			uint32 light_count_;
			uint32 shadow_light_count_;
			uint32 cascaded_shadow_light_count_;
			light_properties_t light_properties_[max_lights_];
			light_split_region_t light_split_regions_[MAX_SPLIT_SHADOW_LIGHTS][max_cascades_];
			shadow_map_properties_t shadow_map_properties_[max_shadow_lights_];
			handle_t shadow_maps_[max_shadow_lights_];
			matrix44_t light_world_[max_lights_];
			matrix44_t light_view_[max_lights_];
			matrix44_t light_proj_[max_lights_];
			matrix44_t light_view_proj_[max_lights_];
			vector4_t light_positions_[max_lights_];
			vector4_t light_directions_[max_lights_];

			// Basic render state
			uint32 window_width_;
			uint32 window_height_;
			Rect view_port_rect_;
			render_state_t::render_target_t render_target_type_;
			handle_t render_target_;
			render_state_t::depth_stencil_t depth_stencil_type_;
			handle_t depth_stencil_;
			depth_test_t depth_test_;
			bool depth_write_;
			bool wire_frame_;
			cull_mode_t cull_mode_;
			vector4_t global_ambient_color_;
			shader_program_t::variant_t shader_variant_;

			// Actions
			bool clear_render_target_;
			vector4_t render_target_clear_value_;
			bool clear_depth_stencil_;
			float depth_clear_value_;		
		};


	} // namespace core

} // namespace maki
