#pragma once
#include <d3d11.h>
#include "Maki.h"

namespace maki {
	namespace d3d {

		bool __failed(HRESULT hr);

	#if _DEBUG
	#	define MAKI_D3D_FAILED(x) __failed(x)
	#else
	#	define MAKI_D3D_FAILED FAILED
	#endif

		extern const char *attr_to_semantic_string[core::attribute_max + 1];
		extern uint32_t attr_to_semantic_index[core::attribute_max + 1];

		extern uint8_t normalize_attribute[core::attribute_max + 1];

		// First index is type, second index is component count, third index is boolean 0 or 1 indicating if value should be normed
		extern const DXGI_FORMAT type_and_count_and_norm_to_format[core::attribute_type_max + 1][5][2];

		extern const D3D11_PRIMITIVE_TOPOLOGY indices_per_face_to_geometry_type[4];
	
		extern const DXGI_FORMAT bytes_per_index_to_format[5];
	
		extern const DXGI_FORMAT channels_to_format[5];

	} // namespace d3d

} // namespace maki
