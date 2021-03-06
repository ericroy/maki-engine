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
			// Stomp out the archive path in debug mode so we will always load loose files from disk
			archivePath = nullptr;

			// In debug mode, the debugModePathPrefix is used to make all assets load from a modified location
			manifest->debugModePathAdjustment = debugModePathAdjustment;
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

			if(archivePath == nullptr) {
#if _DEBUG
				Console::Info("Mounted loose archive rooted at: %s", (debugModePathAdjustment + manifest->commonPathPrefix).c_str());
#else
				Console::Info("Mounted loose archive rooted at: %s", manifest->commonPathPrefix.c_str());
#endif
			}
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

		Rid AssetLibrary::FullPathToRid(const char *path) const
		{
			const uint32 count = groups.size();
			for(uint32 i = 0; i < count; i++) {
				const std::string &prefix = groups[i].manifest->commonPathPrefix;
				if(strstr(path, prefix.c_str()) == path) {
					Rid rid = groups[i].manifest->PathToRid(path + prefix.length());
					if(rid != RID_NONE) {
						return rid;
					}
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