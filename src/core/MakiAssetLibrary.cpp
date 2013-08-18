#include "core/core_stdafx.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiAssetManifest.h"
#include "core/MakiArchive.h"

namespace Maki
{
	namespace Core
	{

		AssetLibrary::AssetLibrary()
			: totalAssetCount(0)
		{
		}
		
		AssetLibrary::~AssetLibrary()
		{
			const uint32 count = groups.size();
			for(uint32 i = 0; i < count; i++) {
				SAFE_DELETE(groups[i].archive);
				// DON'T delete manifest, we do not own it
			}
		}

		bool AssetLibrary::Mount(AssetManifest *manifest, const char *archivePath)
		{
#if _DEBUG
			archivePath = nullptr;
#endif
			Group group;
			group.manifest = manifest;
			group.manifest->SetRidStart(totalAssetCount);

			if(archivePath != nullptr) {
				group.archive = new Archive();
				if(!group.archive->Load(archivePath, totalAssetCount)) {
					delete group.archive;
					Console::Error("Failed to mount archive: %s", archivePath);
					return false;
				}
				Console::Info("Mounted archive: %s", archivePath);
			}
			
			totalAssetCount += group.manifest->GetCount();
			groups.push_back(group);

			Console::Info("Mounted loose archive rooted at: %s", manifest->GetCommonPathPrefix());
			return true;
		}

		Rid AssetLibrary::PathToRid(const char *path) const
		{
			const uint32 count = groups.size();
			for(uint32 i = 0; i < count; i++) {
				Rid rid = groups[i].manifest->PathToRid(path);
				if(rid != RID_NONE) {
					return rid;
				}
			}
			return RID_NONE;
		}

		char *AssetLibrary::AllocRead(Rid rid, uint32 *bytesRead) const
		{
			if(rid == RID_NONE) {
				return nullptr;
			}

			const uint32 count = groups.size();
			for(uint32 i = 0; i < count; i++) {
				const Group &group = groups[i];
				if(group.manifest->Contains(rid)) {
					if(group.archive != nullptr) {
						return group.archive->AllocRead(rid, bytesRead);
					}
					return group.manifest->AllocRead(rid, bytesRead);
				}
			}
			return nullptr;
		}

	} // namespace Core

} // namespace Maki