#pragma once
#include "core/core_stdafx.h"

#if MAKI_USE_D3D

#include "core/MakiVertexFormat.h"

#include <d3d11.h>

namespace Maki
{
	namespace Core
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

			// First index is type, second index is component count
			extern const DXGI_FORMAT typeAndCountToFormat[Core::VertexFormat::DataTypeCount][5];

			extern const D3D11_PRIMITIVE_TOPOLOGY indicesPerFaceToGeometryType[4];
	
			extern const DXGI_FORMAT bytesPerIndexToFormat[5];
	
			extern const DXGI_FORMAT channelsToFormat[5];

		} // namespace D3D

	} // namespace Core

} // namespace Maki

#endif