#pragma once
#include "core/core_stdafx.h"
#include "core/MakiMaterial.h"
#include "core/MakiManager.h"

namespace Maki
{
	namespace Core
	{
		class MaterialManager;
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Manager<Material, MaterialManager>;

		class MAKI_CORE_API MaterialManager : public Manager<Material, MaterialManager>
		{
		public:
			static Handle DuplicateIfShared(Handle handle);

		public:
			static const int32 DEFAULT_SIZE = 64;

		public:
			MaterialManager(uint32 size = DEFAULT_SIZE);	
			virtual ~MaterialManager();	
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);

		private:
			void Reload(Material *mat);
		};

	} // namespace Core

} // namespace Maki
