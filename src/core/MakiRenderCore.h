#pragma once
#include <mutex>
#include "core/MakiThread.h"
#include "core/MakiSafeQueue.h"
#include "core/MakiRenderPayload.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiRenderState.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiDrawCommandList.h"

namespace maki {
	namespace core {

		class vertex_format_t;
		class material_t;

		class render_core_t : public thread_t {
		public:
			enum type_t {
				type_d3d = 0,
				type_ogl,
			};

		public:
			render_core_t() = default;
			virtual ~render_core_t() = default;
			void run();

			// GPU resource creation, updates, destruction
			// These all acquire the execution mutex_ for the render core
			virtual void *upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32_t vertex_count, char *index_data, uint32_t face_count, uint8_t indices_per_face, uint8_t bytes_per_index, bool dynamic, bool length_changed) = 0;
			virtual void free_buffer(void *buffer) = 0;
			virtual bool create_shader_program(shader_program_t *s) = 0;
			virtual void delete_shader_program(shader_program_t *s) = 0;
			virtual bool create_texture(texture_t *t, char *data, uint32_t data_length) = 0;
			virtual bool create_empty_texture(texture_t *t, uint8_t channels) = 0;
			virtual bool create_render_target(texture_t *t) = 0;
			virtual bool create_depth_texture(texture_t *t) = 0;
			virtual void write_to_texture(texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data) = 0;
			virtual void delete_texture(texture_t *t) = 0;

		protected:
			virtual void init() = 0;
			virtual void present() = 0;
			virtual void draw(const render_state_t &state, const draw_command_list_t &commands) = 0;

			template<class Derived>
			void generic_draw(const render_state_t &state, const draw_command_list_t &commands);

			void set_per_frame_constants(const core::render_state_t &state, const core::shader_t *s, char *buffer);
			void set_per_object_constants(const core::shader_t *s, char *buffer, const core::matrix44_t &model, const core::matrix44_t &model_view, const core::matrix44_t &model_view_projection);
			void bind_material_constants(const core::shader_t *s, bool is_vertex_shader, char *buffer, const core::material_t *mat);

		public:
			safe_queue_t<render_payload_t> input;
			safe_queue_t<render_payload_t> output;

		protected:
			uint32_t window_width_;
			uint32_t window_height_;
		};


		template<class Derived>
		void render_core_t::generic_draw(const render_state_t &state, const draw_command_list_t &commands) {
			Derived *derived = static_cast<Derived *>(this);
			derived->acquire_context();

			// Resize screen buffers if necessary
			if(window_width_ != state.window_width || window_height_ != state.window_height) {
				console_t::info("Renderer resizing to %dx%d", state.window_width, state.window_height);
				derived->set_render_target_and_depth_stencil(render_state_t::render_target_null_, HANDLE_NONE, render_state_t::depth_stencil_null_, HANDLE_NONE);
				derived->resized(state.window_width, state.window_height);
				window_width_ = state.window_width;
				window_height_ = state.window_height;
			}

			derived->set_render_target_and_depth_stencil(state.render_target_type, state.render_target, state.depth_stencil_type, state.depth_stencil);
			derived->set_viewport(state.view_port_rect);
			derived->set_depth_state(state.depth_test, state.depth_write);
			derived->set_rasterizer_state(state.cull_mode, state.wire_frame);

			derived->clear(state.clear_render_target, state.render_target_clear_value.vals_, state.clear_depth_stencil, state.depth_clear_value);
						
			uint32_t current_translucency_type = draw_command_t::translucency_type_opaque_;
			derived->set_blend_state(false);

			void *current_buffer = nullptr;
			uint32_t current_layer = 0;
			
			handle_t current_vertex_format = HANDLE_NONE;
			handle_t current_shader_program = HANDLE_NONE;		
			handle_t current_texture_set = HANDLE_NONE;
			handle_t current_mesh = HANDLE_NONE;
			handle_t current_material = HANDLE_NONE;

			bool set_layout = false;

			for(uint32_t i = 0; i < commands.count_; ++i) {
				const draw_command_list_t::value_entry_t &ve = commands.values_[commands.keys_[i].index_];
				const draw_command_t *dc = &ve.draw_command_;
				const matrix44_t &matrix = ve.m_;

				const vertex_format_t *vf = vertex_format_manager_t::get(dc->vertex_format_);
				const shader_program_t *base_shader = shader_program_manager_t::get(dc->shader_program_);
			
				assert(base_shader->variant_ == shader_program_t::variant_normal_ && "can only get variants_ from a normal shader program");
				const shader_program_t *shader = base_shader;
				if(state.shader_variant != shader_program_t::variant_normal_) {
					handle_t h = base_shader->variants_[state.shader_variant-1];
					if(h == HANDLE_NONE) {
						// No such variant for this shader, skip
						continue;
					}
					shader = shader_program_manager_t::get(h);
				}

				if(current_translucency_type != dc->fields_.translucency_type_) {
					if(dc->fields_.translucency_type_ == draw_command_t::translucency_type_translucent_) {
						// Enable blending, disable depth write
						derived->set_depth_state(state.depth_test, false);
						derived->set_blend_state(true);
					} else {
						// Disable blending, restore depth write
						derived->set_depth_state(state.depth_test, state.depth_write);
						derived->set_blend_state(false);
					}
					current_translucency_type = dc->fields_.translucency_type_;
				}

				if(current_vertex_format != dc->vertex_format_) {
					current_vertex_format = dc->vertex_format_;
					set_layout = true;
				}

				if(current_shader_program != dc->shader_program_) {
				
					// Unbind all textures_ from current shader
					if(current_shader_program != HANDLE_NONE)
						derived->unbind_all_textures();

					// Bind the new shader and set per-frame constants
					derived->bind_shaders(shader);
					current_shader_program = dc->shader_program_;
					if(shader->vertex_shader_.frame_uniform_buffer_location_ != -1)
						derived->set_per_frame_vertex_shader_constants(state, shader);
					if(shader->pixel_shader_.frame_uniform_buffer_location_ != -1)
						derived->set_per_frame_pixel_shader_constants(state, shader);
					derived->bind_shadow_maps(shader, state);

					current_material = HANDLE_NONE;
					current_texture_set = HANDLE_NONE;
					current_buffer = nullptr;
					set_layout = true;
				}

				// get or create the input layout for this vertexformat+vertexshader combination
				if(set_layout) {
					if(shader->input_attribute_count_ != vf->attr_count_)
						console_t::warning("shader_t takes %u input attributes, but current vertex format has %u", shader->input_attribute_count_, vf->attr_count_);
					derived->set_input_layout(shader, vf);
					set_layout = false;
				}

				if(current_material != dc->material_) {
					material_t *mat = material_manager_t::get(dc->material_);
					if(shader->vertex_shader_.material_uniform_buffer_location_ != -1)
						derived->set_material_vertex_shader_constants(shader, mat);
					if(shader->pixel_shader_.material_uniform_buffer_location_ != -1)
						derived->set_material_pixel_shader_constants(shader, mat);

					current_material = dc->material_;
					if(current_texture_set != dc->texture_set_) {
						derived->bind_textures(shader, texture_set_manager_t::get(dc->texture_set_));
						current_texture_set = dc->texture_set_;
					}
				}
			
				if(shader->vertex_shader_.object_uniform_buffer_location_ != -1 || shader->pixel_shader_.object_uniform_buffer_location_ != -1) {
					matrix44_t mv = state.view * matrix;
					matrix44_t mvp = state.projection * mv;

					if(shader->vertex_shader_.object_uniform_buffer_location_ != -1)
						derived->set_per_object_vertex_shader_constants(state, shader, matrix, mv, mvp);
					if(shader->pixel_shader_.object_uniform_buffer_location_ != -1)
						derived->set_per_object_pixel_shader_constants(state, shader, matrix, mv, mvp);
				}

				// Bind buffers again if they have changed
				void *b = mesh_manager_t::get(dc->mesh_)->get_buffer();
				if(current_buffer != b) {
					derived->bind_buffer(b, vf);
					current_buffer = b;
				}

				// Actually submit the vertex/index data
				derived->draw_buffer(b);
			}

			derived->bind_buffer(nullptr, nullptr);

			if(current_shader_program != HANDLE_NONE) {
				derived->unbind_all_textures();
				derived->bind_shaders(nullptr);
			}

			derived->set_render_target_and_depth_stencil(render_state_t::render_target_null_, HANDLE_NONE, render_state_t::depth_stencil_null_, HANDLE_NONE);
			derived->release_context();
		}


	} // namespace core
} // namespace maki
