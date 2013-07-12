#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{
	
		class AssetManifest;
		class Archive;

		class MAKI_CORE_API AssetLibrary
		{
		private:
			struct MAKI_CORE_API Group
			{
				Group() : manifest(nullptr), archive(nullptr) {}
				AssetManifest *manifest;
				Archive *archive;
			};

		public:
			AssetLibrary();
			virtual ~AssetLibrary();

			bool Mount(AssetManifest *manifest, const char *archivePath = nullptr);
			Rid PathToRid(const char *path) const;
			char *AllocRead(Rid rid, uint32 *bytesRead = nullptr) const;

		private:
			uint32 totalAssetCount;

#pragma warning(disable:4251)
			std::vector<Group> groups;
#pragma warning(default:4251)
		};

		MAKI_CORE_API_EXTERN template class MAKI_CORE_API std::vector<AssetLibrary::Group>;

	} // namespace Core

} // namespace Maki