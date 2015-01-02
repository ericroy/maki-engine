#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLRenderCore.h"
#include "ogl/MakiOGLGPUTypes.h"
#include "ogl/MakiOGLCommon.h"
#include <algorithm>

using namespace maki::core;

#if MAKI_USE_OGL_MUTEX
#define MAKI_ACQUIRE_OGL_MUTEX std::lock_guard<std::mutex> lock(mutex_);
#else
#define MAKI_ACQUIRE_OGL_MUTEX
#endif

namespace maki
{
	namespace ogl
	{

		#if defined(_WIN32) || defined(_WIN64)
		void APIENTRY OGLDebugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
		#else
		void OGLDebugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
		#endif
		{
			console_t::info("GL DEBUG: %s", message);
		}

		ogl_render_core_t::ogl_render_core_t(window_t *window, const config_t *config)
			:	render_core_t(),
			window(window),
			main_thread_context_(nullptr),
			render_thread_context_(nullptr),
			frame_buffer_(0),
			vao_(0),
			default_render_target_(0),
			default_depth_stencil_(0),
			current_render_target_(0),
			current_render_target_type_(render_state_t::render_target_default_),
			current_depth_stencil_(0),
			current_depth_stencil_type_(render_state_t::depth_stencil_default_),
			depth_write_enabled_(true),
			blend_enabled_(false),
			current_depth_test_(render_state_t::depth_test_less_),
			current_cull_mode_(render_state_t::cull_mode_back_),
			debug_output_(false)
		{
			int32 displayIndex = SDL_GetWindowDisplayIndex(window->window_);
			int32 modeCount = SDL_GetNumDisplayModes(displayIndex);
			SDL_DisplayMode mode;
			bool found = false;
			for(int32 i = 0; i < modeCount; i++) {
				SDL_GetDisplayMode(displayIndex, i, &mode);
				bool resMatch = mode.w == window->width_ && mode.h == window->height_;
				console_t::info("Supported mode: %dx%d @ %d Hz %s", mode.w, mode.h, mode.refresh_rate, resMatch ? "<<< Found!" : "");
				if(resMatch) {
					found = true;
				}
			}
			if(!found)
			{
				console_t::warning("Mode not supported: %dx%d", window->width_, window->height_);
			}
			
			vsync_ = config->get_bool("engine.vsync", false);

			int32 major = config->get_int("ogl.major_version", 3);
			int32 minor = config->get_int("ogl.minor_version", 1);

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
			SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
			
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			if(config->get_bool("ogl.require_hw_accel", false)) {
				console_t::info("Requiring OpenGL hardware acceleration");
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_ACCELERATED_VISUAL);
			}
			if(config->get_bool("ogl.debug_context", false)) {
				console_t::info("Requesting OpenGL debug context");
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
			}


			console_t::info("Creating OpenGL %d.%d contexts", major, minor);

			main_thread_context_ = SDL_GL_CreateContext(window->window_);
			if(main_thread_context_ == nullptr) {
				console_t::error("Failed to create OpenGL main context: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
			
			render_thread_context_ = SDL_GL_CreateContext(window->window_);
			if(render_thread_context_ == nullptr) {
				console_t::error("Failed to create OpenGL render context: %s", SDL_GetError());
				SDL_ClearError();
			}
			
			if(SDL_GL_MakeCurrent(window->window_, main_thread_context_) != 0) {
				console_t::error("Failed to make OpenGL main context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			define_gl_functions();

#if defined(_WIN32) || defined(_WIN64)
			debug_output_ = config->get_bool("ogl.debug_messages", false);
			if(debug_output_ && glDebugMessageCallback != nullptr) {
				console_t::info("Registering for OpenGL debug messages");
				// Register debug callback for main thread's context
				glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
#endif
		}

		ogl_render_core_t::~ogl_render_core_t() {
			glDeleteVertexArrays(1, &vao_);
			glDeleteFramebuffers(1, &frame_buffer_);
			SDL_GL_DeleteContext(main_thread_context_);
			SDL_GL_DeleteContext(render_thread_context_);
			console_t::info("OpenGL renderer destroyed");
		}

		void ogl_render_core_t::init()
		{
			MAKI_ACQUIRE_OGL_MUTEX
			if(SDL_GL_MakeCurrent(window->window_, render_thread_context_) != 0) {
				console_t::error("Failed to make ogl render context current: %s", SDL_GetError());
				SDL_ClearError();
			}

			SDL_GL_SetSwapInterval(vsync_ ? 1 : 0);

#if defined(_WIN32) || defined(_WIN64)
			if(debug_output_ && glDebugMessageCallback != nullptr) {
				// Register debug callback for render thread's context
				glDebugMessageCallback(OGLDebugMessageHandler, nullptr);
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			}
#endif

			
			// Set initial state:
			window_width_ = 1;
			window_height_ = 1;
			glViewport(0, 0, window_width_, window_height_);

			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			glEnable(GL_DEPTH_TEST);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			
			glEnable(GL_BLEND);


			// Render a blank frame so we don't see a flash of white on startup
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			SDL_GL_SwapWindow(window->window_);

			glGenFramebuffers(1, &frame_buffer_);

			// Using one global VAO, because I read in some valve presentation that switching VAOs is typically slower than 
			// just making glVertexAttribPointer calls each time.  I should probably test this out...
			glGenVertexArrays(1, &vao_);
			glBindVertexArray(vao_);
		}

		void ogl_render_core_t::resized(uint32 newWidth, uint32 newHeight)
		{
		}

		void ogl_render_core_t::present()
		{
			MAKI_ACQUIRE_OGL_MUTEX
			SDL_GL_SwapWindow(window->window_);
		}



		// Resource creation, deletion, modification:

		void *ogl_render_core_t::upload_buffer(void *buffer, vertex_format_t *vf, char *vertex_data, uint32 vertex_count, char *index_data, uint32 face_count, uint8 indices_per_face, uint8 bytes_per_index, bool dynamic, bool length_changed)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			buffer_t *b = (buffer_t *)buffer;
			bool reuse = b != nullptr;

			if(reuse) {
				if(length_changed) {
					b->delete_buffers();
					glGenBuffers(2, b->vbos_);
				}
			} else {
				b = new buffer_t();
				memset(b, 0, sizeof(buffer_t));
				glGenBuffers(2, b->vbos_);
			}

			b->vertex_count_ = vertex_count;
			b->face_count_ = face_count;
			b->indices_per_face_ = indices_per_face;
			b->bytes_per_index_ = bytes_per_index;

			assert(indices_per_face > 0 && indices_per_face <= 3);
			b->geometry_type_ = indices_per_face_to_geometry_type[indices_per_face];
		
			assert(bytes_per_index > 0 && bytes_per_index <= 4 && bytes_per_index != 3);
			b->index_data_type_ = bytes_per_index_to_format[bytes_per_index];

			glBindBuffer(GL_ARRAY_BUFFER, b->vbos_[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->vbos_[1]);

			int32 stride = vf->get_stride();
			if(reuse && !length_changed) {
				glBufferSubData(GL_ARRAY_BUFFER, 0, stride*vertex_count, vertex_data);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bytes_per_index*indices_per_face*face_count, index_data);
			} else {
				glBufferData(GL_ARRAY_BUFFER, stride*vertex_count, vertex_data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes_per_index*indices_per_face*face_count, index_data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}

			// Must finish here to ensure that vbos_ have actually received the data before the render thread tries to draw with them
			glFinish();
			
			return (void *)b;
		}

		void ogl_render_core_t::free_buffer(void *buffer)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(buffer != nullptr) {
				buffer_t *b = (buffer_t *)buffer;
				b->delete_buffers();
				delete b;
			}
		}


		bool ogl_render_core_t::create_shader(GLenum shaderType, shader_t *s)
		{
			gpu_shader_t *gs = nullptr;
			if(shaderType == GL_VERTEX_SHADER) {
				gs = new gpu_vertex_shader_t();
			} else {
				gs = new gpu_pixel_shader_t();
			}

			gs->sh_ = glCreateShader(shaderType);
			if(MAKI_OGL_FAILED()) { MAKI_SAFE_DELETE(gs); return false; }

			int32 length = (int32)s->program_data_bytes_;
			glShaderSource(gs->sh_, 1, (const GLchar **)&s->program_data_, &length);
			if(MAKI_OGL_FAILED()) { MAKI_SAFE_DELETE(gs); return false; }
			
			glCompileShader(gs->sh_);
			if(MAKI_OGL_FAILED()) { MAKI_SAFE_DELETE(gs); return false; }

			GLint info_log_length;
			glGetShaderiv(gs->sh_, GL_INFO_LOG_LENGTH, &info_log_length);
			if(info_log_length > 1) {
				char *buffer = new char[info_log_length+1];
				glGetShaderInfoLog(gs->sh_, info_log_length, NULL, buffer);
				console_t::info("Program info log:");
				console_t::info(buffer);
				delete[] buffer;
			}

			GLint compileStatus;
			glGetShaderiv(gs->sh_, GL_COMPILE_STATUS, &compileStatus);
			if(MAKI_OGL_FAILED()) { MAKI_SAFE_DELETE(gs); return false; }
			if(compileStatus == GL_FALSE) {
				console_t::error("Failed to compile glsl %s shader", shaderType == GL_FRAGMENT_SHADER ? "pixel" : "vertex");
				MAKI_SAFE_DELETE(gs);
				return false;
			}

			s->handle_ = (intptr_t)gs;
			return true;
		}

		bool ogl_render_core_t::create_shader_program(shader_program_t *s)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(!create_shader(GL_VERTEX_SHADER, &s->vertex_shader_)) {
				MAKI_OGL_FAILED();
				return false;
			}
			if(!create_shader(GL_FRAGMENT_SHADER, &s->pixel_shader_)) {
				MAKI_OGL_FAILED();
				return false;
			}

			GLuint program = glCreateProgram();
			glAttachShader(program, (GLuint)((gpu_shader_t *)s->pixel_shader_.handle_)->sh_);
			glAttachShader(program, (GLuint)((gpu_shader_t *)s->vertex_shader_.handle_)->sh_);
			for(uint32 i = 0; i < vertex_format_t::attribute_count_; i++) {
				glBindAttribLocation(program, i, attribute_name[i]);
			}
			glLinkProgram(program);
			if(MAKI_OGL_FAILED()) { glDeleteProgram(program); return false; }
			
			GLint info_log_length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
			if(info_log_length > 1) {
				char *buffer = new char[info_log_length+1];
				glGetProgramInfoLog(program, info_log_length, NULL, buffer);
				console_t::info("Program info log:");
				console_t::info(buffer);
				delete[] buffer;
			}

			GLint linkStatus;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if(MAKI_OGL_FAILED()) { glDeleteProgram(program); return false; }
			if(linkStatus == GL_FALSE) {
				console_t::error("Failed to link glsl program");
				glDeleteProgram(program);
				return false;
			}
			assert(glIsProgram(program));


			gpu_vertex_shader_t *gvs = (gpu_vertex_shader_t *)s->vertex_shader_.handle_;

			// Regarding the nastiness below:
			// Most of this info is supposed to be provided in the mshad files that describe the shader.  However,
			// since glsl shaders are not meant to be pre-compiled, we need to resolve this stuff at runtime.
			//
			// This is totally gross since it modifies the cpu-side shader object.

			s->vertex_shader_.frame_uniform_buffer_location_ = glGetUniformBlockIndex(program, "enginePerFrame");
			if(s->vertex_shader_.frame_uniform_buffer_location_ != -1) {
				memset(s->vertex_shader_.engine_frame_uniform_locations_, 0xff, sizeof(s->vertex_shader_.engine_frame_uniform_locations_));
				memset(s->pixel_shader_.engine_frame_uniform_locations_, 0xff, sizeof(s->pixel_shader_.engine_frame_uniform_locations_));

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.frame_uniform_buffer_location_, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertex_shader_.engine_frame_uniform_bytes_ = uniformBlockSize;

				glGenBuffers(1, &gvs->ubo_per_frame_);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_per_frame_);
				glBufferData(GL_UNIFORM_BUFFER, s->vertex_shader_.engine_frame_uniform_bytes_, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertex_shader_.frame_uniform_buffer_location_, uniform_buffer_frame_);

				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.frame_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);

				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertex_shader_.frame_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					shader_t::frame_uniform_t uni = shader_t::get_frame_uniform_by_name(nameBuffer);
					if(s->vertex_shader_.engine_frame_uniform_locations_[uni] == -1) {
						s->vertex_shader_.engine_frame_uniform_locations_[uni] = offsets[i];
						s->pixel_shader_.engine_frame_uniform_locations_[uni] = offsets[i];
					}
				}
				MAKI_SAFE_DELETE(nameBuffer);
			}

			s->vertex_shader_.object_uniform_buffer_location_ = glGetUniformBlockIndex(program, "enginePerObject");
			if(s->vertex_shader_.object_uniform_buffer_location_ != -1) {
				memset(s->vertex_shader_.engine_object_uniform_locations_, 0xff, sizeof(s->vertex_shader_.engine_object_uniform_locations_));
				memset(s->pixel_shader_.engine_object_uniform_locations_, 0xff, sizeof(s->pixel_shader_.engine_object_uniform_locations_));

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.object_uniform_buffer_location_, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertex_shader_.engine_object_uniform_bytes_ = uniformBlockSize;

				glGenBuffers(1, &gvs->ubo_per_object_);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_per_object_);
				glBufferData(GL_UNIFORM_BUFFER, s->vertex_shader_.engine_object_uniform_bytes_, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertex_shader_.object_uniform_buffer_location_, uniform_buffer_object_);
				
				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.object_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);
				
				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertex_shader_.object_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					shader_t::object_uniform_t uni = shader_t::get_object_uniform_by_name(nameBuffer);
					if(s->vertex_shader_.engine_object_uniform_locations_[uni] == -1) {
						s->vertex_shader_.engine_object_uniform_locations_[uni] = offsets[i];
						s->pixel_shader_.engine_object_uniform_locations_[uni] = offsets[i];
					}
				}
				MAKI_SAFE_DELETE(nameBuffer);
			}

			s->vertex_shader_.material_uniform_buffer_location_ = glGetUniformBlockIndex(program, "material");
			if(s->vertex_shader_.material_uniform_buffer_location_ != -1) {
				s->vertex_shader_.material_uniform_locations_.clear();
				s->pixel_shader_.material_uniform_locations_.clear();

				GLint uniformBlockSize = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.material_uniform_buffer_location_, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
				s->vertex_shader_.material_uniform_bytes_ = uniformBlockSize;

				glGenBuffers(1, &gvs->ubo_material_);
				glBindBuffer(GL_UNIFORM_BUFFER, gvs->ubo_material_);
				glBufferData(GL_UNIFORM_BUFFER, s->vertex_shader_.material_uniform_bytes_, 0, GL_STREAM_DRAW);
				glUniformBlockBinding(program, s->vertex_shader_.material_uniform_buffer_location_, uniform_buffer_material_);
				
				GLint activeUniformCount = 0;
				glGetActiveUniformBlockiv(program, s->vertex_shader_.material_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniformCount);

				std::vector<GLint> indices(activeUniformCount, 0);
				std::vector<GLint> offsets(activeUniformCount, 0);
				std::vector<GLint> nameLengths(activeUniformCount, 0);

				glGetActiveUniformBlockiv(program, s->vertex_shader_.material_uniform_buffer_location_, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_OFFSET, offsets.data());
				glGetActiveUniformsiv(program, activeUniformCount, (GLuint *)indices.data(), GL_UNIFORM_NAME_LENGTH, nameLengths.data());

				GLint maxNameLength = *std::max_element(nameLengths.begin(), nameLengths.end());
				char *nameBuffer = new char[maxNameLength+1];

				for(int32 i = 0; i < activeUniformCount; i++) {
					GLint size = 0;
					GLenum type = 0;
					glGetActiveUniform(program, indices[i], maxNameLength+1, nullptr, &size, &type, nameBuffer);

					char *bracket = strchr(nameBuffer, '[');
					if(bracket != nullptr) {
						*bracket = 0;
					}

					int32 index = s->vertex_shader_.find_material_constant_location(nameBuffer);
					if(index == -1) {
						shader_t::material_uniform_location_t uni(offsets[i], nameBuffer);
						s->vertex_shader_.material_uniform_locations_.push_back(uni);
						s->pixel_shader_.material_uniform_locations_.push_back(uni);
					}
				}
				MAKI_SAFE_DELETE(nameBuffer);
			}

			if(MAKI_OGL_FAILED()) { glDeleteProgram(program); return false; }

			int32 largestBuffer = std::max(std::max(s->vertex_shader_.material_uniform_bytes_, s->vertex_shader_.engine_object_uniform_bytes_), s->vertex_shader_.engine_frame_uniform_bytes_);
			gvs->scratch_buffer_ = (char *)allocator_t::malloc(largestBuffer, 16);

			// Lookup texture sampler locations
			// Arbitrarily, we'll decide to store the sampler locations in the vertex shader's array
			char buffer[32];
			for(uint32 i = 0; i < shadow_map_slot_index_start; i++) {
				sprintf(buffer, "uSampler%d", i);
				GLint location = glGetUniformLocation(program, buffer);
				gvs->texture_sampler_locations_[i] = location;
			}
			for(uint32 i = 0; i < core::render_state_t::max_lights_; i++) {
				sprintf(buffer, "uShadowSampler[%d]", i);
				GLint location = glGetUniformLocation(program, buffer);
				gvs->texture_sampler_locations_[shadow_map_slot_index_start+i] = location;
			}

			s->handle_ = (intptr_t)program;
			return true;
		}

	

		bool ogl_render_core_t::create_empty_texture(texture_t *t, uint8 channels)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			if(channels == 0 || channels > 4 || channels == 3) {
				console_t::error("Unsupported number of channels in image: %d", channels);
				return false;
			}

			GLuint tex_ = 0;
			glGenTextures(1, &tex_);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			glBindTexture(GL_TEXTURE_2D, tex_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, channels, t->width_, t->height_, 0, channels_to_format[channels], GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->tex_ = tex_;
			t->handle_ = (intptr_t)gtex;
			return true;	
		}

		bool ogl_render_core_t::create_render_target(texture_t *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GLuint tex_ = 0;
			glGenTextures(1, &tex_);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			glBindTexture(GL_TEXTURE_2D, tex_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width_, t->height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->tex_ = tex_;
			t->handle_ = (intptr_t)gtex;
			return true;
		}

		bool ogl_render_core_t::create_depth_texture(texture_t *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			GLuint tex_ = 0;
			glGenRenderbuffers(1, &tex_);
			glBindRenderbuffer(GL_RENDERBUFFER, tex_);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, t->width_, t->height_);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->tex_ = tex_;
			t->handle_ = (intptr_t)gtex;
			return true;
		}

		bool ogl_render_core_t::create_texture(texture_t *t, char *data, uint32 dataLength)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			const void *dataOut = nullptr;
			unsigned long dataLengthOut = 0;
            uint32 format = 0;
			uint32 mipLevels = 0;
			int32 ret = MOJODDS_getTexture(data, dataLength, &dataOut, &dataLengthOut, &format, &t->width_, &t->height_, &mipLevels);
			if(ret == 0) {
				console_t::error("Failed to mojo-load dds file");
				return false;
			}

			GLuint tex_ = 0;
			glGenTextures(1, &tex_);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			glBindTexture(GL_TEXTURE_2D, tex_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			switch(format)
			{
			case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, t->width_, t->height_, 0, dataLengthOut, dataOut);
				break;
			case GL_RGB:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->width_, t->height_, 0, GL_RGB, GL_UNSIGNED_BYTE, dataOut);
				break;
			case GL_RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->width_, t->height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataOut);
				break;
			case GL_LUMINANCE:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t->width_, t->height_, 0, GL_RED, GL_UNSIGNED_BYTE, dataOut);
				break;
			default:
				console_t::error("Failed to load texture, not a supported internal pixel format");
				glDeleteTextures(1, &tex_); return false;
			}
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			glGenerateMipmap(GL_TEXTURE_2D);
			if(MAKI_OGL_FAILED()) { glDeleteTextures(1, &tex_); return false; }

			gpu_texture_t *gtex = new gpu_texture_t();
			gtex->tex_ = tex_;
			t->handle_ = (intptr_t)gtex;
			return true;
		}

		void ogl_render_core_t::write_to_texture(texture_t *t, int32 dst_x, int32 dst_y, int32 src_x, int32 src_y, uint32 src_width, uint32 src_height, uint32 src_pitch, uint8 channels, char *src_data)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			gpu_texture_t *gtex = (gpu_texture_t *)t->handle_;
			glBindTexture(GL_TEXTURE_2D, gtex->tex_);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, src_pitch/channels);

			GLenum format = channels_to_format[channels];

			// Find the data at point (src_x, src_y) in the source image
			char *p = src_data + (src_y*src_pitch) + (src_x*channels);

			glTexSubImage2D(GL_TEXTURE_2D, 0, dst_x, dst_y, (GLsizei)src_width, (GLsizei)src_height, format, GL_UNSIGNED_BYTE, p);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		}

		void ogl_render_core_t::delete_shader_program(shader_program_t *s)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			console_t::info("Deleting shader: <rid %u> <variant %d>", s->rid_, s->variant_);

			gpu_shader_t *gvs = (gpu_shader_t *)s->vertex_shader_.handle_;
			MAKI_SAFE_DELETE(gvs);
			s->vertex_shader_.handle_ = (intptr_t)nullptr;

			gpu_shader_t *gps = (gpu_shader_t *)s->pixel_shader_.handle_;
			MAKI_SAFE_DELETE(gps);
			s->pixel_shader_.handle_ = (intptr_t)nullptr;
		}

		void ogl_render_core_t::delete_texture(texture_t *t)
		{
			MAKI_ACQUIRE_OGL_MUTEX

			gpu_texture_t *gtex = (gpu_texture_t *)t->handle_;
			MAKI_SAFE_DELETE(gtex);
			t->handle_ = (intptr_t)nullptr;
		}


	} // namespace ogl

} // namespace maki
