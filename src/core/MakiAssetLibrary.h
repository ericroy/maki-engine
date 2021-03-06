#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{
	
		class AssetManifest;
		class Archive;

		class AssetLibrary
		{
		private:
			struct Group
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
			Rid FullPathToRid(const char *path) const;
			char *AllocRead(Rid rid, uint32 *bytesRead = nullptr) const;

		public:
			// Allow a path prefix to be applied for all assets, but only in debug mode
			std::string debugModePathAdjustment;

		private:
			uint32 totalAssetCount;
			std::vector<Group> groups;
		};

		

	} // namespace Core

} // namespace Maki