#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiMesh.h"

namespace Maki
{
	namespace Core
	{

		class MeshManager : public Manager<Mesh, MeshManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 64;

		public:
			MeshManager(uint32 size = DEFAULT_SIZE);	
			virtual ~MeshManager();	
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki
