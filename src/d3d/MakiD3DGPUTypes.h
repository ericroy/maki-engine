#pragma once
#include <d3d11.h>

namespace maki
{
	namespace core
	{
		class vertex_format_t;

	} // namespace core

	namespace d3d
	{

		struct buffer_t
		{
			ID3D11Buffer *vbos_[2];
			uint32 vertex_count_;
			uint32 face_count_;
			uint32 indices_per_face_;
			uint32 bytes_per_index_;
			int32 index_data_type_;
			int32 geometry_type_;
		};

		struct gpu_vertex_shader_t
		{
		private:
			struct input_layout_t
			{
				uint64 vertex_format_key_;
				ID3D11InputLayout *input_layout_;
			};

		public:
			gpu_vertex_shader_t(uint32 vertex_formats_per_vertex_shader);
			~gpu_vertex_shader_t();
			ID3D11InputLayout *get_or_create_input_layout(ID3D11Device *device_, const core::vertex_format_t *vf);
		
			
			ID3D11VertexShader *vs_;
			ID3D11Buffer *per_frame_constants_;
			ID3D11Buffer *per_object_constants_;
			ID3D11Buffer *material_constants_;
			input_layout_t *input_layouts_;
			uint32 input_layout_capacity_;
			uint32 input_layout_count_;
			char *blob_;
			uint32 blob_size_;
		};

		struct gpu_pixel_shader_t
		{
			gpu_pixel_shader_t();
			~gpu_pixel_shader_t();
			
			ID3D11PixelShader *ps_;
			ID3D11Buffer *per_frame_constants_;
			ID3D11Buffer *per_object_constants_;
			ID3D11Buffer *material_constants_;
		};

		struct gpu_texture_t
		{
			gpu_texture_t();
			~gpu_texture_t();

			ID3D11ShaderResourceView *shader_resource_view_;
			ID3D11SamplerState *sampler_state_;

			// Only set if this was created as a depth target
			ID3D11DepthStencilView *depth_stencil_view_;

			// Only set if this was created as a render target
			ID3D11RenderTargetView *render_target_view_;
		};

	} // namespace d3d

} // namespace maki
