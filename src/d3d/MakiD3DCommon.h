#pragma once
#include "d3d/d3d_stdafx.h"

namespace Maki
{
	namespace D3D
	{

		bool _Failed(HRESULT hr);

	#if _DEBUG
	#	define MAKI_D3D_FAILED(x) _Failed(x)
	#else
	#	define MAKI_D3D_FAILED FAILED
	#endif

		extern const char *attrToSemanticString[Core::VertexFormat::AttributeCount];
		extern uint32 attrToSemanticIndex[Core::VertexFormat::AttributeCount];

		extern uint8 normalizeAttribute[Core::VertexFormat::AttributeCount];

		// First index is type, second index is component count, third index is boolean 0 or 1 indicating if value should be normed
		extern const DXGI_FORMAT typeAndCountAndNormToFormat[Core::VertexFormat::DataTypeCount][5][2];

		extern const D3D11_PRIMITIVE_TOPOLOGY indicesPerFaceToGeometryType[4];
	
		extern const DXGI_FORMAT bytesPerIndexToFormat[5];
	
		extern const DXGI_FORMAT channelsToFormat[5];

	} // namespace D3D

} // namespace Maki
