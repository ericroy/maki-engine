#pragma once
#include <d3d11.h>

namespace maki
{
	namespace d3d
	{

		bool __failed(HRESULT hr);

	#if _DEBUG
	#	define MAKI_D3D_FAILED(x) __failed(x)
	#else
	#	define MAKI_D3D_FAILED FAILED
	#endif

		extern const char *attr_to_semantic_string[core::vertex_format_t::attribute_count_];
		extern uint32 attr_to_semantic_index[core::vertex_format_t::attribute_count_];

		extern uint8 normalize_attribute[core::vertex_format_t::attribute_count_];

		// First index is type, second index is component count, third index is boolean 0 or 1 indicating if value should be normed
		extern const DXGI_FORMAT type_and_count_and_norm_to_format[core::vertex_format_t::data_type_count_][5][2];

		extern const D3D11_PRIMITIVE_TOPOLOGY indices_per_face_to_geometry_type[4];
	
		extern const DXGI_FORMAT bytes_per_index_to_format[5];
	
		extern const DXGI_FORMAT channels_to_format[5];

	} // namespace d3d

} // namespace maki
