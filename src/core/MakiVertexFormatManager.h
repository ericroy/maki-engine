#pragma once
#include "core/core_stdafx.h"
#include "core/MakiVertexFormat.h"
#include "core/MakiManager.h"

namespace Maki
{
	namespace Core
	{
		class VertexFormatManager;
		

		class VertexFormatManager : public Manager<VertexFormat, VertexFormatManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 8;

		public:
			inline Handle FindOrAdd(const VertexFormat &vf)
			{
				Handle vertexFormat = Find(vf);
				if(vertexFormat == HANDLE_NONE) {
					return Add(vf);
				}
				return vertexFormat;
			}

		public:
			VertexFormatManager(uint32 size = DEFAULT_SIZE);	
			virtual ~VertexFormatManager();
		};

	} // namespace Core

} // namespace Maki
