#include "core/MakiAssetManifest.h"
#include "core/MakiConsole.h"

namespace maki {
	namespace core {

		asset_manifest_t::asset_manifest_t(uint32_t asset_count, rid_t *rid_array, const char **path_array, const char *common_path_prefix)
			: rid_start_(0),
			length_(asset_count),
			rid_array_(rid_array),
			path_array_(path_array),
			common_path_prefix(common_path_prefix) {
		}

		void asset_manifest_t::set_rid_start(uint32_t rid_start) {
			// Renumber all rids, starting at <rid_start_>
			rid_start_ = rid_start;
			for(uint32_t i = 0; i < length_; i++)
				rid_array_[i] = rid_start + i;
		}

		rid_t asset_manifest_t::path_to_rid(const char *path) const {
			string full_path = common_path_prefix + path;
			for(uint32_t i = 0; i < length_; i++) {
				if(full_path == path_array_[i])
					return MAKI_TO_RID(rid_array_[i]);
			}
			return RID_NONE;
		}

		array_t<char> asset_manifest_t::alloc_read(rid_t rid) const {
			if(rid == RID_NONE)
				return nullptr;
			const char *path = get_path(rid);

#if MAKI_DEBUG
			string full_path = debug_mode_path_adjustment + path;
			path = full_path.c_str();
#endif

			FILE *fp = fopen(path, "rb");
			if(fp == nullptr) {
				console_t::error("Failed to open resource: %s (%s)", path, strerror(errno));
				return nullptr;
			}
		
			fseek(fp, 0, SEEK_END);
			size_t length = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			array_t<char> buffer(length + 1);
			fread(buffer.data(), sizeof(uint8_t), length, fp);
			fclose(fp);
			buffer[length] = 0;
			return buffer;
		}

	} // namespace core
} // namespace maki
