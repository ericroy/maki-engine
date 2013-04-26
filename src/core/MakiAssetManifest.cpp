#include "core/core_stdafx.h"
#include "core/MakiAssetManifest.h"
#include <sstream>

namespace Maki
{

	AssetManifest::AssetManifest(uint32 assetCount, Rid *ridArray, const char **pathArray, const char *commonPathPrefix)
		: ridStart(0),
		count(assetCount),
		ridArray(ridArray),
		pathArray(pathArray),
		commonPathPrefix(commonPathPrefix)
	{
	}
		
	AssetManifest::~AssetManifest()
	{
	}

	void AssetManifest::SetRidStart(uint32 ridStart)
	{
		// Renumber all rids, starting at <ridStart>
		this->ridStart = ridStart;
		for(uint32 i = 0; i < count; i++) {
			ridArray[i] = ridStart+i;
		}
	}

	Rid AssetManifest::PathToRid(const char *path) const
	{
		std::string fullPath = commonPathPrefix + path;
		for(uint32 i = 0; i < count; i++) {
			if(fullPath == pathArray[i]) {
				return TO_RID(i);
			}
		}
		return RID_NONE;
	}

	char *AssetManifest::AllocRead(Rid rid, uint32 *bytesRead) const
	{
		if(rid == RID_NONE) {
			return false;
		}
		const char *path = GetPath(rid);

		FILE *fp = nullptr;
		if(fopen_s(&fp, path, "rb") != 0) {
			Console::Error("Failed to open resource: %s", path);
			return nullptr;
		}
		
		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *buffer = (char *)Allocator::Malloc(size+1);
		fread(buffer, sizeof(uint8), size, fp);
		fclose(fp);
		buffer[size] = 0;

		if(bytesRead != nullptr) {
			*bytesRead = size;
		}
		return buffer;
	}

} // namespace Maki