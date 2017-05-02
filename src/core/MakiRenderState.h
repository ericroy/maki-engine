#pragma once
#include "core/MakiTypes.h"
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
				float attenuation = 0.0f;
				float spot_factor = 0.0f;
				float fov = 0.0f;
				uint32_t flags = 0;
			};

			struct shadow_map_properties_t {
				vector2_t size;
				float pad[2];
			};

			struct camera_split_distances_t {
				float splits[max_cascades - 1] = {};
			};

			struct __declspec(align(MAKI_SIMD_ALIGN)) light_split_region_t : public aligned_t<MAKI_SIMD_ALIGN> {
				matrix44_t view_proj;
				vector4_t width_height_near_far;
			};

		public:
			render_state_t() = default;
			~render_state_t() {
				clear();
			}
			void copy(const render_state_t &s) {
				*this = s;
			}
			void clear() {
				for (auto &tex : shadow_maps)
					tex.release();
				render_target.release();
				depth_stencil.release();
			}

		private:
			render_state_t(const render_state_t &other) {}

		public:
			// Camera data
			matrix44_t view;
			matrix44_t projection;
			camera_split_distances_t camera_split_distances;
			vector4_t camera_width_height_near_far;

			// light_t and shadow data
			uint32_t light_count = 0;
			uint32_t shadow_light_count = 0;
			uint32_t cascaded_shadow_light_count = 0;
			light_properties_t light_properties[max_lights];
			light_split_region_t light_split_regions[max_split_shadow_lights][max_cascades];
			shadow_map_properties_t shadow_map_properties[max_shadow_lights];
			ref_t<texture_t> shadow_maps[max_shadow_lights];
			matrix44_t light_world[max_lights];
			matrix44_t light_view[max_lights];
			matrix44_t light_proj[max_lights];
			matrix44_t light_view_proj[max_lights];
			vector4_t light_positions[max_lights];
			vector4_t light_directions[max_lights];

			// Basic render state
			uint32_t window_width;
			uint32_t window_height;
			rect_t view_port_rect;
			render_state_t::render_target_t render_target_type = render_target_default;
			ref_t<texture_t> render_target;
			render_state_t::depth_stencil_t depth_stencil_type = depth_stencil_default;
			ref_t<texture_t> depth_stencil;
			depth_test_t depth_test = depth_test_less;
			bool depth_write = true;
			bool wire_frame = false;
			cull_mode_t cull_mode = cull_mode_back;
			vector4_t global_ambient_color;
			shader_program_variant_t shader_variant = shader_program_variant_normal;

			// Actions
			bool clear_render_target = false;
			vector4_t render_target_clear_value;
			bool clear_depth_stencil = false;
			float depth_clear_value = 1.0f;		
		};


	} // namespace core
} // namespace maki
