#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "ogl/MakiOGLCommon.h"
#include "ogl/MakiOGLGPUTypes.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#define MAKI_USE_OGL_MUTEX 0

namespace maki
{
	namespace core
	{
		
		class window_t;
		class input_layout_cache_t;
		class vertex_format_t;
		class shader_program_t;
		class texture_set_t;
		class texture_manager_t;
		class material_t;
		class texture_t;
		class config_t;

	} // namespace core

	namespace ogl
	{


		class ogl_render_core_t : public core::render_core_t
		{
		private:
			enum uniform_buffer_t
			{
				uniform_buffer_frame_ = 0,
				uniform_buffer_object_,
				uniform_buffer_material_
			};

		public:
			ogl_render_core_t(core::window_t *window, const core::config_t *config);
			virtual ~ogl_render_core_t();
			void init();
			inline void draw(const core::render_state_t &state, const core::draw_command_list_t &commands) { generic_draw<ogl_render_core_t>(state, commands); }
			void present();
			void resized(uint32_t width, uint32_t height);

			// GPU resource creation / destruction
			void *upload_buffer(void *buffer, core::vertex_format_t *vf, char *vertex_data, uint32_t vertex_count, char *index_data, uint32_t face_count, uint8_t indices_per_face, uint8_t bytes_per_index, bool dynamic, bool length_changed);
			void free_buffer(void *buffer);
			bool create_shader_program(core::shader_program_t *s);
			void delete_shader_program(core::shader_program_t *s);
			bool create_texture(core::texture_t *t, char *data, uint32_t dataLength);
			bool create_empty_texture(core::texture_t *t, uint8_t channels);
			bool create_render_target(core::texture_t *t);
			bool create_depth_texture(core::texture_t *t);
			void write_to_texture(core::texture_t *t, int32_t dst_x, int32_t dst_y, int32_t src_x, int32_t src_y, uint32_t src_width, uint32_t src_height, uint32_t src_pitch, uint8_t channels, char *src_data);
			void delete_texture(core::texture_t *t);

			// Non-virtual interface
			inline void acquire_context();
			inline void release_context();
			inline void set_render_target_and_depth_stencil(core::render_state_t::render_target_t renderTargetType, handle_t renderTarget, core::render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil);
			inline void set_viewport(const core::rect_t &viewPortRect);
			inline void clear(bool clear_render_target, const float clear_color_values[4], bool clear_depth_stencil, float clear_depth_value);
			inline void set_depth_state(core::render_state_t::depth_test_t depth_test, bool depth_write);
			inline void set_rasterizer_state(core::render_state_t::cull_mode_t cull_mode, bool wire_frame);
			inline void set_blend_state(bool enabled);
			inline void unbind_all_textures();
			inline void bind_shaders(const core::shader_program_t *shader);
			inline void set_per_frame_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader);
			inline void set_per_frame_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader);
			inline void bind_shadow_maps(const core::shader_program_t *shader, const core::render_state_t &state);
			inline void set_input_layout(const core::shader_program_t *shader, const core::vertex_format_t *vf);
			inline void set_material_vertex_shader_constants(const core::shader_program_t *shader, const core::material_t *mat);
			inline void set_material_pixel_shader_constants(const core::shader_program_t *shader, const core::material_t *mat);
			inline void bind_textures(const core::shader_program_t *shader, const core::texture_set_t *ts);
			inline void set_per_object_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp);
			inline void set_per_object_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp);
			inline void bind_buffer(void *buffer, const core::vertex_format_t *vf);
			inline void draw_buffer(void *buffer);

		private:
			bool create_shader(GLenum shaderType, core::shader_t *ps);

		private:
			core::window_t *window;
			SDL_GLContext render_thread_context_;
			SDL_GLContext main_thread_context_;

			uint32_t window_width_;
			uint32_t window_height_;

			GLuint frame_buffer_;

			GLuint default_render_target_;
			GLuint default_depth_stencil_;

			GLuint current_render_target_;
			core::render_state_t::render_target_t current_render_target_type_;

			GLuint current_depth_stencil_;
			core::render_state_t::depth_stencil_t current_depth_stencil_type_;

			bool depth_write_enabled_;
			bool blend_enabled_;
			core::render_state_t::depth_test_t current_depth_test_;
			core::render_state_t::cull_mode_t current_cull_mode_;
			
			bool vsync_;
			bool debug_output_;
			std::mutex mutex_;


			GLuint vao_;
		};







		inline void ogl_render_core_t::acquire_context()
		{
#if MAKI_USE_OGL_MUTEX
			mutex_.lock();
#endif
		}

		inline void ogl_render_core_t::release_context()
		{
#if MAKI_USE_OGL_MUTEX
			mutex_.unlock();
#endif
		}

		inline void ogl_render_core_t::set_render_target_and_depth_stencil(core::render_state_t::render_target_t renderTargetType, handle_t renderTarget, core::render_state_t::depth_stencil_t depth_stencil_type, handle_t depth_stencil)
		{
			using namespace core;

			// Default framebuffer
			GLuint fbo = 0;

			current_render_target_ = 0;
			current_render_target_type_ = renderTargetType;
			if(renderTargetType == render_state_t::render_target_default_) {
				assert(depth_stencil_type == render_state_t::depth_stencil_default_ && "If one of render target or depth stencil is 'default', then the other must be too");
				current_render_target_ = default_render_target_;
			} else if(renderTargetType == render_state_t::render_target_custom_) {
				fbo = frame_buffer_;
				current_render_target_ = ((gpu_texture_t *)texture_manager_t::get(renderTarget)->handle_)->tex_;
				if(current_render_target_ == 0) {
					console_t::error("Tried to set render target to an invalid texture");
				}
			} else {
				fbo = frame_buffer_;
			}
			
			current_depth_stencil_ = 0;
			current_depth_stencil_type_ = depth_stencil_type;
			if(depth_stencil_type == render_state_t::depth_stencil_default_) {
				assert(renderTargetType == render_state_t::render_target_default_ && "If one of render target or depth stencil is 'default', then the other must be too");
				current_depth_stencil_ = default_depth_stencil_;
			} else if(depth_stencil_type == render_state_t::depth_stencil_custom_) {
				fbo = frame_buffer_;
				current_depth_stencil_ = ((gpu_texture_t *)texture_manager_t::get(depth_stencil)->handle_)->tex_;
				if(current_depth_stencil_ == 0) {
					console_t::error("Tried to set depth stencil to an invalid texture");
				}
			} else {
				fbo = frame_buffer_;
			}			
			
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			if(fbo != 0) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, current_render_target_, 0);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, current_depth_stencil_);
			}
		}

		inline void ogl_render_core_t::set_viewport(const core::rect_t &viewPortRect)
		{
			glViewport(0, 0, (GLsizei)viewPortRect.get_width(), (GLsizei)viewPortRect.get_height());
		}

		inline void ogl_render_core_t::clear(bool clear_render_target, const float clear_color_values[4], bool clear_depth_stencil, float clear_depth_value)
		{
			using namespace core;

			bool tempEnableDepthWrite = false;

			GLuint clearFlags = 0;
			if(clear_render_target && current_render_target_type_ != render_state_t::render_target_null_) {
				glClearColor(clear_color_values[0], clear_color_values[1], clear_color_values[2], clear_color_values[3]);
				clearFlags |= GL_COLOR_BUFFER_BIT;
			}
			if(clear_depth_stencil && current_depth_stencil_type_ != render_state_t::depth_stencil_null_) {
				tempEnableDepthWrite = !depth_write_enabled_;
				glClearDepth(clear_depth_value);
				clearFlags |= GL_DEPTH_BUFFER_BIT;
			}
			if(clearFlags != 0) {
				if(tempEnableDepthWrite) {
					glDepthMask(GL_TRUE);
					glClear(clearFlags);
					glDepthMask(GL_FALSE);
				} else {
					glClear(clearFlags);
				}
			}
		}

		inline void ogl_render_core_t::set_depth_state(core::render_state_t::depth_test_t depth_test, bool depth_write)
		{
			using namespace core;

			if(depth_write_enabled_ != depth_write) {
				glDepthMask(depth_write ? GL_TRUE : GL_FALSE);
				depth_write_enabled_ = depth_write;
			}

			if(current_depth_test_ != depth_test) {
				if(depth_test == render_state_t::depth_test_disabled_) {
					glDisable(GL_DEPTH_TEST);
				} else if(current_depth_test_ == render_state_t::depth_test_disabled_) {
					glEnable(GL_DEPTH_TEST);
				}

				switch(depth_test) {
				case render_state_t::depth_test_less_:
					glDepthFunc(GL_LESS);
					break;
				case render_state_t::depth_test_equal_:
					glDepthFunc(GL_EQUAL);
					break;
				case render_state_t::depth_test_less_equal_:
					glDepthFunc(GL_LEQUAL);
					break;
				default:
					break;
				}

				current_depth_test_ = depth_test;
			}
		}

		inline void ogl_render_core_t::set_rasterizer_state(core::render_state_t::cull_mode_t cull_mode, bool wire_frame)
		{
			using namespace core;

			if(current_cull_mode_ != cull_mode) {
				if(cull_mode == render_state_t::cull_mode_none_) {
					glDisable(GL_CULL_FACE);
				} else if(current_cull_mode_ == render_state_t::cull_mode_none_) {
					glEnable(GL_CULL_FACE);
				}

				switch(cull_mode) {
				case render_state_t::cull_mode_front_:
					glCullFace(GL_FRONT);
					break;
				case render_state_t::cull_mode_back_:
					glCullFace(GL_BACK);
					break;
				default:
					break;
				}

				current_cull_mode_ = cull_mode;
			}
		}

		inline void ogl_render_core_t::set_blend_state(bool enabled)
		{
			if(blend_enabled_ != enabled) {
				if(enabled) {
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				} else {
					glDisable(GL_BLEND);
				}

				blend_enabled_ = enabled;
			}
		}

		inline void ogl_render_core_t::unbind_all_textures()
		{
			for(uint32_t i = 0; i < shadow_map_slot_index_start+core::render_state_t::max_lights_; i++) {
				glActiveTexture(GL_TEXTURE0+i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		inline void ogl_render_core_t::bind_shaders(const core::shader_program_t *shader)
		{
			if(shader != nullptr) {
				assert(glIsProgram((GLuint)shader->handle_));
				glUseProgram((GLuint)shader->handle_);
			} else {
				glUseProgram(0);
			}
		}

		inline void ogl_render_core_t::set_per_frame_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader)
		{
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;

			set_per_frame_constants(state, &shader->vertex_shader_, gvs->scratch_buffer_);
			set_per_frame_constants(state, &shader->pixel_shader_, gvs->scratch_buffer_);
			
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_per_frame_);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniform_buffer_frame_, gvs->ubo_per_frame_);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertex_shader_.engine_frame_uniform_bytes_, gvs->scratch_buffer_);
		}

		inline void ogl_render_core_t::set_per_frame_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader)
		{
		}

		inline void ogl_render_core_t::bind_shadow_maps(const core::shader_program_t *shader, const core::render_state_t &state)
		{
			using namespace core;

			// Recall that we arbitrarily decided to store texture sampler locations in the vertex shader (rather than pixel shader)
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;

			for(uint8_t i = 0; i < render_state_t::max_shadow_lights_; i++) {
				int32_t sampler_idex = shadow_map_slot_index_start+i;
				int32_t location = gvs->texture_sampler_locations_[sampler_idex];
				if(location >= 0) {
					glActiveTexture(GL_TEXTURE0+sampler_idex);
					glUniform1i(gvs->texture_sampler_locations_[sampler_idex], sampler_idex);
					if(state.shadow_maps_[i] != HANDLE_NONE) {
						gpu_texture_t *gtex = (gpu_texture_t *)texture_manager_t::get(state.shadow_maps_[i])->handle_;
						glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex_);
					} else {
						glBindTexture(GL_TEXTURE_2D, 0);
					}
				}
			}
		}

		inline void ogl_render_core_t::set_input_layout(const core::shader_program_t *shader, const core::vertex_format_t *vf)
		{
			using namespace core;

			for(uint32_t i = 0; i < vertex_format_t::attribute_count_; i++) {
				vertex_format_t::attribute_t attr = (vertex_format_t::attribute_t)i;
				if(vf->has_attribute(attr)) {
					glEnableVertexAttribArray(attr);
				} else {
					glDisableVertexAttribArray(attr);
				}
			}
		}

		inline void ogl_render_core_t::set_material_vertex_shader_constants(const core::shader_program_t *shader, const core::material_t *mat)
		{
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;

			bind_material_constants(&shader->vertex_shader_, false, gvs->scratch_buffer_, mat);
			bind_material_constants(&shader->pixel_shader_, false, gvs->scratch_buffer_, mat);
			
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_material_);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniform_buffer_material_, gvs->ubo_material_);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertex_shader_.material_uniform_bytes_, gvs->scratch_buffer_);
		}

		inline void ogl_render_core_t::set_material_pixel_shader_constants(const core::shader_program_t *shader, const core::material_t *mat)
		{
		}

		inline void ogl_render_core_t::bind_textures(const core::shader_program_t *shader, const core::texture_set_t *ts)
		{
			using namespace core;

			// Recall that we arbitrarily decided to store texture sampler locations in the vertex shader (rather than pixel shader)
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;
			
			for(uint32_t i = 0; i < ts->texture_count_; i++) {
				int32_t location = gvs->texture_sampler_locations_[i];
				if(location >= 0) {
					const gpu_texture_t *gtex = (gpu_texture_t *)texture_manager_t::get(ts->textures_[i])->handle_;
					glActiveTexture(GL_TEXTURE0+i);
					glUniform1i(location, i);
					glBindTexture(GL_TEXTURE_2D, (GLuint)gtex->tex_);
				}
			}
		}

		inline void ogl_render_core_t::set_per_object_vertex_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp)
		{
			const gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)shader->vertex_shader_.handle_;

			set_per_object_constants(&shader->vertex_shader_, gvs->scratch_buffer_, matrix, mv, mvp);
			set_per_object_constants(&shader->pixel_shader_, gvs->scratch_buffer_, matrix, mv, mvp);
			
			glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_per_object_);
			glBindBufferBase(GL_UNIFORM_BUFFER, uniform_buffer_object_, gvs->ubo_per_object_);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, shader->vertex_shader_.engine_object_uniform_bytes_, gvs->scratch_buffer_);
		}

		inline void ogl_render_core_t::set_per_object_pixel_shader_constants(const core::render_state_t &state, const core::shader_program_t *shader, const core::matrix44_t &matrix, const core::matrix44_t &mv, const core::matrix44_t &mvp)
		{
		}

		inline void ogl_render_core_t::bind_buffer(void *buffer, const core::vertex_format_t *vf)
		{
			using namespace core;

			if(buffer == nullptr) {
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				return;
			}

			const buffer_t *b = (buffer_t *)buffer;
			glBindBuffer(GL_ARRAY_BUFFER, b->vbos_[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos_[1]);

			const int32_t stride = vf->get_stride();
			uint32_t offset = 0;
			for(uint32_t i = 0; i < vertex_format_t::attribute_count_; i++) {
				vertex_format_t::attribute_t attr = (vertex_format_t::attribute_t)i;
				if(vf->has_attribute(attr)) {
					GLint count = vf->get_data_count(attr);
					vertex_format_t::data_type_t type = vf->get_data_type(attr);
					glVertexAttribPointer(attr, count, type_to_gl_type[type], normalize_attribute[attr], stride, (GLvoid *)offset);
					offset += count * vertex_format_t::data_type_sizes_[type];
				}
			}
		}

		inline void ogl_render_core_t::draw_buffer(void *buffer)
		{
			const buffer_t *b = (buffer_t *)buffer;
			glDrawElements(b->geometry_type_, b->face_count_*b->indices_per_face_, b->index_data_type_, nullptr);
		}


		// End of non-virtual interface

	} // namespace ogl

} // namespace maki
