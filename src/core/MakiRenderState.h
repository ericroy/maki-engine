#pragma once
#include "core/MakiRect.h"
#include "core/MakiMatrix44.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiTextureManager.h"


namespace maki {
	namespace core {

		class __declspec(align(MAKI_SIMD_ALIGN)) render_state_t : public aligned_t<MAKI_SIMD_ALIGN>
		{
		public:
			static const int32_t max_lights = 8;
			static const int32_t max_shadow_lights = 2;
			static const int32_t max_split_shadow_lights = 2;
			static const int32_t max_cascades = 5;

			enum render_target_t {
				render_target_null = 0,
				render_target_default,
				render_target_custom
			};
			enum depth_stencil_t {
				depth_stencil_null = 0,
				depth_stencil_default,
				depth_stencil_custom
			};
			enum cull_mode_t {
				cull_mode_none = 0,
				cull_mode_front,
				cull_mode_back
			};
			enum depth_test_t {
				depth_test_disabled = 0,
				depth_test_less,
				depth_test_equal,
				depth_test_less_equal
			};
			enum light_type_t {
				light_type_directional = 0,
				light_type_point,
				light_type_spot,
				light_type_count
			};

			enum light_flag_t {
				light_flag_type_directional = 1 << light_type_directional,
				light_flag_type_point = 1 << light_type_point,
				light_flag_type_spot = 1 << light_type_spot,

				light_flag_on = 1 << 3,
				light_flag_shadow = 1 << 4,
				light_flag_split_shadow = 1 << 5,
			};

			// Must match the light properties struct defined in shaders
			struct light_properties_t {
				vector4_t ambient_color;
				vector4_t diffuse_color;
				vector4_t specular_color;
				vector4_t width_height_near_far;
				float attenuation;
				float spot_factor;
				float fov;
				uint32_t flags;
			};

			struct shadow_map_properties_t {
				vector2_t size;
				float pad[2];
			};

			struct camera_split_distances_t {
				float splits[max_cascades - 1];
			};

			struct __declspec(align(MAKI_SIMD_ALIGN)) light_split_region_t : public aligned_t<MAKI_SIMD_ALIGN> {
				matrix44_t view_proj;
				vector4_t width_height_near_far;
			};

		public:
			render_state_t()
			{
				memset(light_properties_, 0, sizeof(light_properties_));
				render_target_type_ = render_target_default;
				depth_stencil_type_ = depth_stencil_default;
				clear_render_target_ = false;
				clear_depth_stencil_ = false;
				depth_test_ = depth_test_less;
				depth_write_ = true;
				wire_frame_ = false;
				global_ambient_color_ = vector4_t(0.0f);
				cull_mode_ = cull_mode_back;
				shader_variant_ = shader_program_t::variant_normal;
			
				light_count_ = 0;
				shadow_light_count_ = 0;
				cascaded_shadow_light_count_ = 0;

				render_target_ = HANDLE_NONE;
				depth_stencil_ = HANDLE_NONE;
				for(uint32_t i = 0; i < max_shadow_lights; i++) {
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
				for(uint32_t i = 0; i < max_shadow_lights; i++)
					texture_manager_t::add_ref(shadow_maps_[i]);
				texture_manager_t::add_ref(render_target_);
				texture_manager_t::add_ref(depth_stencil_);
			}

			void clear()
			{
				for(uint32_t i = 0; i < max_shadow_lights; i++)
					texture_manager_t::free(shadow_maps_[i]);
				texture_manager_t::free(render_target_);
				texture_manager_t::free(depth_stencil_);
			}

		private:
			render_state_t(const render_state_t &other) {}

		public:
			// Camera data
			matrix44_t view_;
			matrix44_t projection_;
			camera_split_distances_t camera_split_distances_;
			vector4_t camera_width_height_near_far_;

			// light_t and shadow data
			uint32_t light_count_;
			uint32_t shadow_light_count_;
			uint32_t cascaded_shadow_light_count_;
			light_properties_t light_properties_[max_lights];
			light_split_region_t light_split_regions_[max_split_shadow_lights][max_cascades];
			shadow_map_properties_t shadow_map_properties_[max_shadow_lights];
			handle_t shadow_maps_[max_shadow_lights];
			matrix44_t light_world_[max_lights];
			matrix44_t light_view_[max_lights];
			matrix44_t light_proj_[max_lights];
			matrix44_t light_view_proj_[max_lights];
			vector4_t light_positions_[max_lights];
			vector4_t light_directions_[max_lights];

			// Basic render state
			uint32_t window_width_;
			uint32_t window_height_;
			rect_t view_port_rect_;
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
