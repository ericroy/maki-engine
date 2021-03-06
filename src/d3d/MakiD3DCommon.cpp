#include "d3d/d3d_stdafx.h"
#include "d3d/MakiD3DCommon.h"

using namespace Maki::Core;

namespace Maki
{
	namespace D3D
	{

		bool _Failed(HRESULT hr) {
			if(FAILED(hr)) {
				LPCSTR errorText = nullptr;
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorText, 0, nullptr);
				if(errorText != nullptr) {
					Console::Error("D3D: %s", errorText);
					LocalFree((HLOCAL)errorText);
					errorText = nullptr;
				} else {
					Console::Error("FormatMessageA failed: %d", GetLastError());
				}
				return true;
			}
			return false;
		}

		const char *attrToSemanticString[VertexFormat::AttributeCount] = {
			"POSITION", "NORMAL", "TANGENT", "COLOR", "COLOR", "TEXCOORD", "TEXCOORD"
		};

		uint32 attrToSemanticIndex[VertexFormat::AttributeCount] = { 0, 0, 0, 0, 1, 0, 1 };

		uint8 normalizeAttribute[Core::VertexFormat::AttributeCount] = { 0, 0, 0, 1, 1, 0, 0 };

		// First index is type, second index is component count
		const DXGI_FORMAT typeAndCountAndNormToFormat[VertexFormat::DataTypeCount][5][2] = {
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

		const D3D11_PRIMITIVE_TOPOLOGY indicesPerFaceToGeometryType[4] = {
			D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
			D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		};
	
		const DXGI_FORMAT bytesPerIndexToFormat[5] = {
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_R32_UINT,
		};

		const DXGI_FORMAT channelsToFormat[5] = {
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_A8_UNORM,
			DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT_UNKNOWN,			// 3 channels texture not supported
			DXGI_FORMAT_R8G8B8A8_UNORM
		};

	} // namespace D3D

} // namespace Maki
