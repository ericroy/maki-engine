#include "d3d/MakiD3DGPUTypes.h"
#include "d3d/MakiD3DCommon.h"

using namespace maki::core;

namespace maki {
	namespace d3d {

		gpu_vertex_shader_t::gpu_vertex_shader_t(uint32_t vertex_formats_per_vertex_shader) : input_layouts(vertex_formats_per_vertex_shader) {
		}

		gpu_vertex_shader_t::~gpu_vertex_shader_t() {
			MAKI_SAFE_RELEASE(vs);
			MAKI_SAFE_RELEASE(per_frame_constants);
			MAKI_SAFE_RELEASE(per_object_constants);
			MAKI_SAFE_RELEASE(material_constants);
		}
		
		ID3D11InputLayout *gpu_vertex_shader_t::get_or_create_input_layout(ID3D11Device *device, const vertex_format_t *vf) {
			for (auto &il : input_layouts) {
				if(il.vertex_format_key == vf->comparison_key)
					return il.input_layout;
			}

			// Else create this layout and cache it
			assert(input_layout_count < input_layouts.length() && "Too many vertex formats for this vertex shader");

			uint32_t offset = 0;
			uint32_t index = 0;
			D3D11_INPUT_ELEMENT_DESC layout_data[core::attribute_max + 1];
			for(uint8_t i = 0; i <= core::attribute_max; i++) {
				vertex_format_t::attribute_t attr = (vertex_format_t::attribute_t)i;
				if(vf->has_attribute(attr)) {
					auto attr_type = vf->get_attribute_type(attr);
					auto elem_count = vf->get_attribute_element_count(attr);

					layout_data[index].SemanticName = attr_to_semantic_string[attr];
					layout_data[index].SemanticIndex = attr_to_semantic_index[attr];
					layout_data[index].Format = type_and_count_and_norm_to_format[attr_type][elem_count][normalize_attribute[attr]];
					layout_data[index].InputSlot = 0;
					layout_data[index].AlignedByteOffset = offset;
					layout_data[index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					layout_data[index].InstanceDataStepRate = 0;

					offset += elem_count * core::attribute_type_sizes[attr_type];
					index++;
				}
			}

			if(MAKI_D3D_FAILED(device_->CreateInputLayout(layout_data, index, blob.data(), blob.length(), &input_layouts[input_layout_count].input_layout))) {
				console_t::error("Failed to create input layout");
				return nullptr;
			}
			input_layouts[input_layout_count].vertex_format_key = vf->comparison_key;
			return input_layouts[input_layout_count++].input_layout;
		}
		

		gpu_pixel_shader_t::~gpu_pixel_shader_t() {
			MAKI_SAFE_RELEASE(ps);
			MAKI_SAFE_RELEASE(per_frame_constants);
			MAKI_SAFE_RELEASE(per_object_constants);
			MAKI_SAFE_RELEASE(material_constants);
		}


		gpu_texture_t::~gpu_texture_t() {
			MAKI_SAFE_RELEASE(shader_resource_view);
			MAKI_SAFE_RELEASE(sampler_state);
			MAKI_SAFE_RELEASE(depth_stencil_view);
			MAKI_SAFE_RELEASE(render_target_view);
		}

	} // namespace d3d

} // namespace maki
