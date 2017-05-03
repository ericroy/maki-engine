#pragma once
#include "core/MakiWindow.h"
#include "core/MakiDrawCommandList.h"
#include "core/MakiRenderCore.h"

namespace maki {
	namespace core {

		class resource_library_t;
		class config_t;
		class vertex_format_t;
		class shader_program_t;
		class shader_t;
		class pixel_shader_t;
		class vertex_shader_t;
		class texture_t;
		class light_t;

		
		namespace {
			const int32_t max_render_payloads = 16;
			const int32_t default_max_draw_commands_per_pass = 1024;
		}
		

		// The renderer_t is the front-end (main thread) counterpart to the back-end (worker thread) render_core_t.
		// It accepts render commands and state changes locally, and then sends them to the render_core_t
		// when you call submit().
		class renderer_t : public aligned_t<MAKI_SIMD_ALIGN> {
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
		
			// state_t actions
			inline void clear_depth(float value);
			inline void clear_color(const vector4_t &rgba);

			// state_t setters
			void set_render_target(render_state_t::render_target_t render_target_type, const ref_t<texture_t> &render_target = nullptr);
			void set_depth_stencil(render_state_t::depth_stencil_t depth_stencil_type, const ref_t<texture_t> &depth_stencil = nullptr);
			inline void set_viewport(const rect_t &rect);
			inline void set_view(const matrix44_t &view);
			inline void set_camera_matrix(const matrix44_t &camera_matrix);
			void set_perspective_projection(const frustum_t &frustum);
			void set_ortho_projection(const frustum_t &frustum);
			inline void set_cull_mode(render_state_t::cull_mode_t mode);
			inline void set_wire_frame_enabled(bool on);
			inline void set_depth_write_enabled(bool on);
			inline void set_depth_test(render_state_t::depth_test_t test);
			inline void set_global_ambient_color(const vector4_t &color);
			inline void set_shader_variant(shader_program_variant_t variant);
	
			inline void set_light_count(uint32_t count, uint32_t shadow_count = 0, uint32_t split_shadow_count = 0);
			void set_light(uint32_t light_index, const render_state_t::light_properties_t *props = nullptr, const render_state_t::shadow_map_properties_t *shad_props = nullptr, const matrix44_t *matrix = nullptr, float fov = 0.0f, const ref_t<texture_t> &depth_buffer = nullptr);
			void set_light_cascade(uint32_t light_index, uint32_t cascade_index, const frustum_t &frustum);
			inline void set_camera_split_distances(uint32_t count, float *split_distances);

			// Getters
			inline window_t *get_window() const;
			inline render_state_t::cull_mode_t get_cull_mode() const;
			inline bool get_wire_frame_enabled() const;
			inline bool get_depth_write_enabled() const;
			inline render_state_t::depth_test_t get_depth_test() const;
			inline const vector4_t &get_global_ambient_color() const;
			inline shader_program_variant_t get_shader_variant() const;
			inline const matrix44_t &get_view() const;
			inline const matrix44_t &get_camera_matrix() const;

			// GPU resource creation, updates, destruction
			// These actions are applied synchonously on the core, so they involve acquiring a mutex_
			inline void *upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32_t vertex_count, char *index_data, uint32_t face_count, uint8_t indices_per_face, uint8_t bytes_per_index, bool dynamic, bool length_changed);
			inline void free_buffer(void *buffer);
			inline bool create_shader_program(shader_program_t *s);
			inline void delete_shader_program(shader_program_t *s);
			inline bool create_texture(texture_t *t, char *data, uint32_t data_length);
			inline bool create_empty_texture(texture_t *t, uint8_t channels);
			inline bool create_render_target(texture_t *t);
			inline bool create_depth_texture(texture_t *t);
			inline void delete_texture(texture_t *t);
			inline void write_to_texture(texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data);
		

		private:
			void prepare_next_render_state();

		protected:
			window_t *window_ = nullptr;
			render_state_t current_;
			uint32_t light_dirty_flags_ = 0;
			render_state_t *state_ = nullptr;
			draw_command_list_t *commands_ = nullptr;
			std::vector<render_state_t *> render_states_;
			std::vector<draw_command_list_t *> command_lists_;
			render_core_t *core_ = nullptr;

		private:
			// Memebers that are used to provide convenience functions but are not part of the renderstate
			matrix44_t camera_matrix_;
		};






		inline void renderer_t::draw(const draw_command_t &draw_command, const matrix44_t &m) {
			commands_->add(draw_command, m);
		}
		
		inline void renderer_t::clear_depth(float value) {
			current_.clear_depth_stencil = true; current_.depth_clear_value = value;
		}
	
		inline void renderer_t::clear_color(const vector4_t &rgba) {
			current_.clear_render_target = true;
			current_.render_target_clear_value = rgba;
		}

		inline void renderer_t::set_viewport(const rect_t &rect) {
			current_.view_port_rect = rect;
		}

		inline void renderer_t::set_view(const matrix44_t &view) {
			current_.view = view;
			matrix44_t::affine_inverse(view, camera_matrix_);
		}

		inline void renderer_t::set_camera_matrix(const matrix44_t &camera_matrix) {
			camera_matrix_ = camera_matrix;
			matrix44_t::affine_inverse(camera_matrix, current_.view);
		}

		inline void renderer_t::set_light_count(uint32_t light_count, uint32_t shadow_light_count, uint32_t cascaded_shadow_light_count) {
			MAKI_ASSERT(light_count <= render_state_t::max_lights);
			MAKI_ASSERT(shadow_light_count <= light_count);
			MAKI_ASSERT(cascaded_shadow_light_count <= shadow_light_count);

			current_.light_count = light_count;
			current_.shadow_light_count = shadow_light_count;
			current_.cascaded_shadow_light_count = cascaded_shadow_light_count;
		}

		inline void renderer_t::set_camera_split_distances(uint32_t split_count, float *split_distances) {
			MAKI_ASSERT(split_count < render_state_t::max_cascades);
			memcpy(current_.camera_split_distances.splits, split_distances, split_count * sizeof(float));
		}

		inline void renderer_t::set_cull_mode(render_state_t::cull_mode_t mode) {
			current_.cull_mode = mode;
		}

		inline void renderer_t::set_wire_frame_enabled(bool on) {
			current_.wire_frame = on;
		}

		inline void renderer_t::set_depth_write_enabled(bool on) {
			current_.depth_write = on;
		}

		inline void renderer_t::set_depth_test(render_state_t::depth_test_t test) {
			current_.depth_test = test;
		}

		inline void renderer_t::set_global_ambient_color(const vector4_t &color) {
			current_.global_ambient_color = color;
		}

		inline void renderer_t::set_shader_variant(shader_program_variant_t variant) {
			current_.shader_variant = variant;
		}
	




		inline window_t *renderer_t::get_window() const {
			return window_;
		}

		inline render_state_t::cull_mode_t renderer_t::get_cull_mode() const {
			return current_.cull_mode;
		}
	
		inline bool renderer_t::get_wire_frame_enabled() const {
			return current_.wire_frame;
		}

		inline bool renderer_t::get_depth_write_enabled() const {
			return current_.depth_write;
		}

		inline render_state_t::depth_test_t renderer_t::get_depth_test() const {
			return current_.depth_test;
		}

		inline const vector4_t &renderer_t::get_global_ambient_color() const {
			return current_.global_ambient_color;
		}

		inline shader_program_variant_t renderer_t::get_shader_variant() const {
			return current_.shader_variant;
		}

		inline const matrix44_t &renderer_t::get_view() const {
			return current_.view;
		}

		inline const matrix44_t &renderer_t::get_camera_matrix() const {
			return camera_matrix_;
		}



		inline void *renderer_t::upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32_t vertex_count, char *index_data, uint32_t face_count, uint8_t indices_per_face, uint8_t bytes_per_index, bool dynamic, bool length_changed) {
			return core_->upload_buffer(buffer, vf, vertex_data, vertex_count, index_data, face_count, indices_per_face, bytes_per_index, dynamic, length_changed);
		}
	
		inline void renderer_t::free_buffer(void *buffer) {
			core_->free_buffer(buffer);
		}

		inline bool renderer_t::create_shader_program(shader_program_t *s) {
			return core_->create_shader_program(s);
		}
	
		inline void renderer_t::delete_shader_program(shader_program_t *s) {
			core_->delete_shader_program(s);
		}

		inline bool renderer_t::create_texture(texture_t *t, char *data, uint32_t data_length) {
			return core_->create_texture(t, data, data_length);
		}

		inline bool renderer_t::create_empty_texture(texture_t *t, uint8_t channels) {
			return core_->create_empty_texture(t, channels);
		}

		inline bool renderer_t::create_render_target(texture_t *t) {
			return core_->create_render_target(t);
		}

		inline bool renderer_t::create_depth_texture(texture_t *t) {
			return core_->create_depth_texture(t);
		}

		inline void renderer_t::delete_texture(texture_t *t) {
			core_->delete_texture(t);
		}

		inline void renderer_t::write_to_texture(texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data) {
			core_->write_to_texture(t, dst_x, dst_y, src_x, src_y, src_width, src_height, src_pitch, channels, src_data);
		}


	} // namespace core
} // namespace maki
