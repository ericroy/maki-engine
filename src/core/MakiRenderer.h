#pragma once
#include "core/core_stdafx.h"
#include "core/MakiWindow.h"
#include "core/MakiDrawCommandList.h"
#include "core/MakiRenderCore.h"

namespace maki
{
	namespace core
	{

		class resource_library_t;
		class config_t;
		class vertex_format_t;
		class shader_program_t;
		class shader_t;
		class pixel_shader_t;
		class vertex_shader_t;
		class texture_t;
		class light_t;

		
		

		// The renderer_t is the front-end (main thread) counterpart to the back-end (worker thread) render_core_t.
		// It accepts render commands and state changes locally, and then sends them to the render_core_t
		// when you call submit().
		class renderer_t : public aligned_t<MAKI_SIMD_ALIGN>
		{
		public:
			static const int32 max_render_payloads_ = 16;
			static const int32 default_max_draw_commands_per_pass_ = 1024;

		public:
			renderer_t(window_t *window, render_core_t *core, const config_t *config);
			virtual ~renderer_t();

			// Basic frame lifecycle
			bool begin();
			inline void draw(const draw_command_t &draw_command, const matrix44_t &m);
			void submit();
			void end();

			// Call this to block until the core has finished all its current tasks.
			// Once this function returns, all the render state objects and draw command
			// lists will once again be in the posession of this renderer object (in the main thread).
			void sync_with_core();
		
			// State actions
			inline void clear_depth(float value);
			inline void clear_color(const vector4_t &rgba);

			// State setters
			void set_render_target_and_depth_stencil(render_state_t::render_target_t render_target_type, handle_t render_target, render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil);
			inline void set_viewport(const Rect &rect);
			inline void set_view(const matrix44_t &view);
			inline void set_camera_matrix(const matrix44_t &camera_matrix);
			void set_perspective_projection(const frustum_t &frustum);
			void set_ortho_projection(const frustum_t &frustum);
			inline void set_cull_mode(render_state_t::cull_mode_t mode);
			inline void set_wire_frame_enabled(bool on);
			inline void set_depth_write_enabled(bool on);
			inline void set_depth_test(render_state_t::depth_test_t test);
			inline void set_global_ambient_color(const vector4_t &color);
			inline void set_shader_variant(shader_program_t::variant_t variant);
	
			inline void set_light_count(uint32 count, uint32 shadow_count = 0, uint32 split_shadow_count = 0);
			void set_light(uint32 light_index, const render_state_t::LightProperties *props = nullptr, const render_state_t::ShadowMapProperties *shad_props = nullptr, const matrix44_t *matrix = nullptr, float fov = 0.0f, handle_t depth_buffer = HANDLE_NONE);
			void set_light_cascade(uint32 light_index, uint32 cascade_index, const frustum_t &frustum);
			inline void set_camera_split_distances(uint32 count, float *split_distances);

			// Getters
			inline window_t *get_window() const;
			inline render_state_t::cull_mode_t get_cull_mode() const;
			inline bool get_wire_frame_enabled() const;
			inline bool get_depth_write_enabled() const;
			inline render_state_t::depth_test_t get_depth_test() const;
			inline const vector4_t &get_global_ambient_color() const;
			inline shader_program_t::variant_t get_shader_variant() const;
			inline const matrix44_t &get_view() const;
			inline const matrix44_t &get_camera_matrix() const;

			// GPU resource creation, updates, destruction
			// These actions are applied synchonously on the core, so they involve acquiring a mutex
			inline void *upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32 vertex_count, char *index_data, uint32 face_count, uint8 indices_per_face, uint8 bytes_per_index, bool dynamic, bool length_changed);
			inline void free_buffer(void *buffer);
			inline bool create_shader_program(shader_program_t *s);
			inline void delete_shader_program(shader_program_t *s);
			inline bool create_texture(texture_t *t, char *data, uint32 data_length);
			inline bool create_empty_texture(texture_t *t, uint8 channels);
			inline bool create_render_target(texture_t *t);
			inline bool create_depth_texture(texture_t *t);
			inline void delete_texture(texture_t *t);
			inline void write_to_texture(texture_t *t, int32 dst_x, int32 dst_y, int32 src_x, int32 src_y, uint32 src_width, uint32 src_height, uint32 src_pitch, uint8 channels, char *src_data);
		

		private:
			void prepare_next_render_state();

		protected:
			window_t *window_;
			render_state_t current_;
			uint32 light_dirty_flags;
			render_state_t *state;
			draw_command_list_t *commands;
			std::vector<render_state_t *> render_states;
			std::vector<draw_command_list_t *> command_lists;
			render_core_t *core_;

		private:
			// Memebers that are used to provide convenience functions but are not part of the renderstate
			matrix44_t camera_matrix_;
		};






		inline void renderer_t::draw(const draw_command_t &draw_command, const matrix44_t &m)
		{
			commands->add(draw_command, m);
		}
		
		inline void renderer_t::clear_depth(float value)
		{
			current_.clear_depth_stencil_ = true; current_.depth_clear_value_ = value;
		}
	
		inline void renderer_t::clear_color(const vector4_t &rgba)
		{
			current_.clear_render_target_ = true;
			current_.render_target_clear_value_ = rgba;
		}

		inline void renderer_t::set_viewport(const Rect &rect)
		{
			current_.view_port_rect_ = rect;
		}

		inline void renderer_t::set_view(const matrix44_t &view)
		{
			current_.view_ = view;
			matrix44_t::affine_inverse(view, camera_matrix_);
		}

		inline void renderer_t::set_camera_matrix(const matrix44_t &camera_matrix)
		{
			camera_matrix_ = camera_matrix;
			matrix44_t::affine_inverse(camera_matrix, current_.view_);
		}

		inline void renderer_t::set_light_count(uint32 light_count, uint32 shadow_light_count, uint32 cascaded_shadow_light_count)
		{
			assert(light_count <= render_state_t::max_lights_);
			assert(shadow_light_count <= light_count);
			assert(cascaded_shadow_light_count <= shadow_light_count);

			current_.light_count_ = light_count;
			current_.shadow_light_count_ = shadow_light_count;
			current_.cascaded_shadow_light_count_ = cascaded_shadow_light_count;
		}

		inline void renderer_t::set_camera_split_distances(uint32 splitCount, float *split_distances)
		{
			assert(splitCount < render_state_t::MAX_CASCADES);
			memcpy(current_.cameraSplitDistances.splits, split_distances, splitCount*sizeof(float));
		}

		inline void renderer_t::set_cull_mode(render_state_t::cull_mode_t mode)
		{
			current_.cull_mode_ = mode;
		}

		inline void renderer_t::set_wire_frame_enabled(bool on)
		{
			current_.wire_frame_ = on;
		}

		inline void renderer_t::set_depth_write_enabled(bool on)
		{
			current_.depth_write_ = on;
		}

		inline void renderer_t::set_depth_test(render_state_t::depth_test_t test)
		{
			current_.depth_test_ = test;
		}

		inline void renderer_t::set_global_ambient_color(const vector4_t &color)
		{
			current_.global_ambient_color_ = color;
		}

		inline void renderer_t::set_shader_variant(shader_program_t::variant_t variant)
		{
			current_.shader_variant_ = variant;
		}
	




		inline window_t *renderer_t::get_window() const
		{
			return window;
		}

		inline render_state_t::cull_mode_t renderer_t::get_cull_mode() const
		{
			return current_.cull_mode_;
		}
	
		inline bool renderer_t::get_wire_frame_enabled() const
		{
			return current_.wire_frame_;
		}

		inline bool renderer_t::get_depth_write_enabled() const
		{
			return current_.depth_write_;
		}

		inline render_state_t::depth_test_t renderer_t::get_depth_test() const
		{
			return current_.depth_test_;
		}

		inline const vector4_t &renderer_t::get_global_ambient_color() const
		{
			return current_.global_ambient_color_;
		}

		inline shader_program_t::variant_t renderer_t::get_shader_variant() const
		{
			return current_.shader_variant_;
		}

		inline const matrix44_t &renderer_t::get_view() const
		{
			return current_.view_;
		}

		inline const matrix44_t &renderer_t::get_camera_matrix() const
		{
			return camera_matrix_;
		}



		inline void *renderer_t::upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32 vertex_count, char *index_data, uint32 face_count, uint8 indices_per_face, uint8 bytes_per_index, bool dynamic, bool length_changed)
		{
			return core_->upload_buffer(buffer, vf, vertex_data, vertex_count, index_data, face_count, indices_per_face, bytes_per_index, dynamic, length_changed);
		}
	
		inline void renderer_t::free_buffer(void *buffer)
		{
			core_->free_buffer(buffer);
		}

		inline bool renderer_t::create_shader_program(shader_program_t *s)
		{
			return core_->create_shader_program(s);
		}
	
		inline void renderer_t::delete_shader_program(shader_program_t *s)
		{
			core_->delete_shader_program(s);
		}

		inline bool renderer_t::create_texture(texture_t *t, char *data, uint32 data_length)
		{
			return core_->create_texture(t, data, data_length);
		}

		inline bool renderer_t::create_empty_texture(texture_t *t, uint8 channels)
		{
			return core_->create_empty_texture(t, channels);
		}

		inline bool renderer_t::create_render_target(texture_t *t)
		{
			return core_->create_render_target(t);
		}

		inline bool renderer_t::create_depth_texture(texture_t *t)
		{
			return core_->create_depth_texture(t);
		}

		inline void renderer_t::delete_texture(texture_t *t)
		{
			core_->delete_texture(t);
		}

		inline void renderer_t::write_to_texture(texture_t *t, int32 dst_x, int32 dst_y, int32 src_x, int32 src_y, uint32 src_width, uint32 src_height, uint32 src_pitch, uint8 channels, char *src_data)
		{
			core_->write_to_texture(t, dst_x, dst_y, src_x, src_y, src_width, src_height, src_pitch, channels, src_data);
		}

	} // namespace core

} // namespace maki
