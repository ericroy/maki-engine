#include "core/core_stdafx.h"
#include "core/MakiAssetManifest.h"
#include <sstream>
#include <string.h>

namespace maki
{
	namespace core
	{

		asset_manifest_t::asset_manifest_t(uint32 asset_count, rid_t *rid_array, const char **path_array, const char *common_path_prefix)
			: rid_start(0),
			count(asset_count),
			rid_array(rid_array),
			path_array(path_array),
			common_path_prefix(common_path_prefix)
		{
		}
		
		asset_manifest_t::~asset_manifest_t()
		{
		}

		void asset_manifest_t::set_rid_start(uint32 rid_start)
		{
			// Renumber all rids, starting at <rid_start>
			this->rid_start = rid_start;
			for(uint32 i = 0; i < count; i++) {
				rid_array[i] = rid_start+i;
			}
		}

		rid_t asset_manifest_t::path_to_rid(const char *path) const
		{
			std::string fullPath = common_path_prefix + path;
			for(uint32 i = 0; i < count; i++) {
				if(fullPath == path_array[i]) {
					return MAKI_TO_RID(rid_array[i]);
				}
			}
			return RID_NONE;
		}

		char *asset_manifest_t::alloc_read(rid_t rid, uint32 *bytesRead) const
		{
			if(rid == RID_NONE) {
				return nullptr;
			}
			const char *path = get_path(rid);

#if _DEBUG
			std::string fullPath = debug_mode_path_adjustment_ + path;
			path = fullPath.c_str();
#endif

			FILE *fp = fopen(path, "rb");
			if(fp == nullptr) {
				console_t::error("Failed to open resource: %s (%s)", path, strerror(errno));
				return nullptr;
			}
		
			fseek(fp, 0, SEEK_END);
			size_t size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			char *buffer = (char *)allocator_t::malloc(size+1);
			fread(buffer, sizeof(uint8), size, fp);
			fclose(fp);
			buffer[size] = 0;

			if(bytesRead != nullptr) {
				*bytesRead = size;
			}
			return buffer;
		}

	} // namespace core

} // namespace maki
