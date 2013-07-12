#include "core/core_stdafx.h"
#include "core/direct3d/MakiD3DCommon.h"

#if MAKI_USE_D3D

namespace Maki
{
	namespace Core
	{
		namespace D3D
		{

			bool _Failed(HRESULT hr) {
				if(FAILED(hr)) {
					LPCSTR errorText = nullptr;
					FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS,
						nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorText, 0, nullptr);
					if(errorText != nullptr) {
					   Console::Error("D3D error: %s", errorText);
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
				"POSITION", "NORMAL", "TANGENT", "COLOR", "TEXCOORD", "TEXCOORD"
			};
			uint32 attrToSemanticIndex[VertexFormat::AttributeCount] = {
				0, 0, 0, 0, 0, 1
			};

			// First index is type, second index is component count
			const DXGI_FORMAT typeAndCountToFormat[VertexFormat::DataTypeCount][5] = {
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT },		// Float
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT },			// UnsignedInt32
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_UINT },					// UnsignedInt16
				{ DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UINT },						// UnsignedInt8
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

	} // namespace Core

} // namespace Maki


#endif