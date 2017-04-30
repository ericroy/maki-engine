#pragma once
#include "ogl/MakiOGLCommon.h"

namespace maki
{
	namespace core
	{
		class vertex_format_t;

	} // namespace core

	namespace ogl
	{

		struct buffer_t
		{
			GLuint vbos_[2];
			uint32_t vertex_count_;
			uint32_t face_count_;
			uint32_t indices_per_face_;
			uint32_t bytes_per_index_;
			GLenum index_data_type_;
			GLenum geometry_type_;

			void delete_buffers();
		};

		struct gpu_shader_t
		{
			gpu_shader_t();
			virtual ~gpu_shader_t();

			GLuint sh_;
		};

		struct gpu_vertex_shader_t : public gpu_shader_t
		{
		public:
			gpu_vertex_shader_t();
			virtual ~gpu_vertex_shader_t();

			GLuint ubo_per_frame_;
			GLuint ubo_per_object_;
			GLuint ubo_material_;
			char *scratch_buffer_;
			GLint texture_sampler_locations_[shadow_map_slot_index_start+core::render_state_t::max_lights_];
		};

		struct gpu_pixel_shader_t : public gpu_shader_t
		{
			gpu_pixel_shader_t() : gpu_shader_t() {}
			virtual ~gpu_pixel_shader_t() {}
		};

		struct gpu_texture_t
		{
			gpu_texture_t();
			~gpu_texture_t();

			GLuint tex_;
		};

	} // namespace ogl

} // namespace maki
