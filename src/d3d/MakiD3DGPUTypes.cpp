#include "d3d/d3d_stdafx.h"
#include "d3d/MakiD3DGPUTypes.h"
#include "d3d/MakiD3DCommon.h"

using namespace maki::core;

namespace maki
{
	namespace d3d
	{

		gpu_vertex_shader_t::gpu_vertex_shader_t(uint32 vertex_formats_per_vertex_shader)
			: blob_(nullptr),
			blob_size_(0),
			vs(nullptr),
			per_frame_constants_(nullptr),
			per_object_constants_(nullptr),
			material_constants_(nullptr),
			input_layouts_(nullptr),
			input_layout_count_(0),
			input_layout_capacity_(vertex_formats_per_vertex_shader)
		{
			input_layouts_ = (input_layout_t *)Allocator::Malloc(sizeof(input_layout_t) * input_layout_capacity_);
			memset(input_layouts_, 0, sizeof(input_layout_t)*input_layout_capacity_);
		}

		gpu_vertex_shader_t::~gpu_vertex_shader_t()
		{
			for(uint32 i = 0; i < input_layout_count_; i++) {
				SAFE_RELEASE(input_layouts_[i].input_layout_);
			}
			SAFE_FREE(input_layouts_);

			SAFE_RELEASE(vs);
			SAFE_FREE(blob_);
			SAFE_RELEASE(per_frame_constants_);
			SAFE_RELEASE(per_object_constants_);
			SAFE_RELEASE(material_constants_);
		}

		ID3D11InputLayout *gpu_vertex_shader_t::get_or_create_input_layout(ID3D11Device *device_, const vertex_format_t *vf) {
			for(uint32 i = 0; i < input_layout_count_; i++) {
				if(input_layouts_[i].vertex_format_key_ == vf->equalityKey) {
					return input_layouts_[i].input_layout_;
				}
			}

			// Else create this layout and cache it
			assert(input_layout_count_ < input_layout_capacity_ && "Too many vertex formats for this vertex shader");

			uint32 offset = 0;
			uint32 index = 0;
			D3D11_INPUT_ELEMENT_DESC layoutData[vertex_format_t::attribute_count_];
			for(uint8 i = 0; i < vertex_format_t::attribute_count_; i++) {
				vertex_format_t::Attribute attr = (vertex_format_t::Attribute)i;
				if(vf->HasAttribute(attr)) {

					vertex_format_t::DataType dataType = vf->GetDataType(attr);
					uint8 dataCount = vf->GetDataCount(attr);

					layoutData[index].SemanticName = attr_to_semantic_string[attr];
					layoutData[index].SemanticIndex = attr_to_semantic_index[attr];
					layoutData[index].Format = type_and_count_and_norm_to_format[dataType][dataCount][normalize_attribute[attr]];
					layoutData[index].InputSlot = 0;
					layoutData[index].AlignedByteOffset = offset;
					layoutData[index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					layoutData[index].InstanceDataStepRate = 0;

					offset += dataCount * vertex_format_t::DataTypeSizes[dataType];
					index++;
				}
			}

			if(MAKI_D3D_FAILED(device_->CreateInputLayout(layoutData, index, blob_, blob_size_, &input_layouts_[input_layout_count_].input_layout_))) {
				console_t::error("Failed to create input layout");
				return nullptr;
			}
			input_layouts_[input_layout_count_].vertex_format_key_ = vf->equalityKey;
			return input_layouts_[input_layout_count_++].input_layout_;
		}
		
			




		gpu_pixel_shader_t::gpu_pixel_shader_t()
			: ps(nullptr),
			per_frame_constants_(nullptr),
			per_object_constants_(nullptr),
			material_constants_(nullptr)
		{
		}

		gpu_pixel_shader_t::~gpu_pixel_shader_t()
		{
			SAFE_RELEASE(ps);
			SAFE_RELEASE(per_frame_constants_);
			SAFE_RELEASE(per_object_constants_);
			SAFE_RELEASE(material_constants_);
		}
			




		gpu_texture_t::gpu_texture_t()
			: shader_resource_view_(nullptr),
			sampler_state_(nullptr),
			depth_stencil_view_(nullptr),
			render_target_view_(nullptr)
		{
		}

		gpu_texture_t::~gpu_texture_t()
		{
			SAFE_RELEASE(shader_resource_view_);
			SAFE_RELEASE(sampler_state_);
			SAFE_RELEASE(depth_stencil_view_);
			SAFE_RELEASE(render_target_view_);
		}

	} // namespace d3d

} // namespace maki
