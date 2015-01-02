#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLGPUTypes.h"

namespace maki
{
	namespace ogl
	{

		void buffer_t::delete_buffers()
		{
			glDeleteBuffers(2, vbos_);
			vbos_[0] = 0;
			vbos_[1] = 0;
		}


		gpu_shader_t::gpu_shader_t() : sh_(0) {}
		gpu_shader_t::~gpu_shader_t() { glDeleteShader(sh_); }


		gpu_vertex_shader_t::gpu_vertex_shader_t()
			: gpu_shader_t(), ubo_per_frame_(0), ubo_per_object_(0), ubo_material_(0), scratch_buffer_(nullptr)
		{
			memset(texture_sampler_locations_, 0xff, sizeof(texture_sampler_locations_));
		}

		gpu_vertex_shader_t::~gpu_vertex_shader_t()
		{
			glDeleteBuffers(1, &ubo_per_frame_);
			glDeleteBuffers(1, &ubo_per_object_);
			glDeleteBuffers(1, &ubo_material_);
			glDeleteShader(sh_);
			MAKI_SAFE_FREE(scratch_buffer_);
		}


		gpu_texture_t::gpu_texture_t()
			: tex_(0)
		{
		}

		gpu_texture_t::~gpu_texture_t()
		{
			glDeleteTextures(1, &tex_);
		}

	} // namespace ogl

} // namespace maki
