#include "d3d/MakiD3DCommon.h"

using namespace maki::core;

namespace maki {
	namespace d3d {

		bool __failed(HRESULT hr) {
			if(FAILED(hr)) {
				LPCSTR error_text = nullptr;
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error_text, 0, nullptr);
				if(error_text != nullptr) {
					console_t::error("D3D: %s", error_text);
					LocalFree((HLOCAL)error_text);
					error_text = nullptr;
				} else {
					console_t::error("FormatMessageA failed: %d", GetLastError());
				}
				return true;
			}
			return false;
		}

		const char *attr_to_semantic_string[attribute_max + 1] = {
			"POSITION", "NORMAL", "TANGENT", "COLOR", "COLOR", "TEXCOORD", "TEXCOORD"
		};

		uint32_t attr_to_semantic_index[attribute_max + 1] = { 0, 0, 0, 0, 1, 0, 1 };

		uint8_t normalize_attribute[attribute_max + 1] = { 0, 0, 0, 1, 1, 0, 0 };

		// First index is type, second index is component count
		const DXGI_FORMAT type_and_count_and_norm_to_format[attribute_type_max + 1][5][2] = {
			// Float
			{
				{DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT},
				{DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT},
				{DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT},
				{DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT}
			},

			// UnsignedInt32
			{
				{DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT},
				{DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_UINT},
				{DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32_UINT},
				{DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT}
			},
			
			// UnsignedInt16
			{
				{DXGI_FORMAT_UNKNOWN,DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_UNORM},
				{DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_UNORM},
				{DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UNORM}
			},

			// UnsignedInt8
			{
				{DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UNORM},
				{DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8_UNORM},
				{DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN},
				{DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM}
			},							
		};

		const D3D11_PRIMITIVE_TOPOLOGY indices_per_face_to_geometry_type[4] = {
			D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
			D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		};
	
		const DXGI_FORMAT bytes_per_index_to_format[5] = {
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R32_UINT,
		};

		const DXGI_FORMAT channels_to_format[5] = {
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_A8_UNORM,
			DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT_UNKNOWN,			// 3 channels texture not supported
			DXGI_FORMAT_R8G8B8A8_UNORM
		};

	} // namespace d3d
} // namespace maki
