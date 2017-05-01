#include "core/MakiAssetLibrary.h"
#include "core/MakiConsole.h"
#include "core/MakiAssetManifest.h"
#include "core/MakiArchive.h"

namespace maki {
	namespace core {
		
		asset_library_t::~asset_library_t() {
			for(auto &group : groups_) {
				// DON'T delete manifest, we do not own it
				MAKI_SAFE_DELETE(group.archive);
			}
		}

		bool asset_library_t::mount(asset_manifest_t *manifest, const char *archive_path) {
#if _DEBUG
			// Stomp out the archive_ path in debug mode so we will always load loose files from disk
			archive_path = nullptr;

			// In debug mode, the path adjustment is used to make all assets load from a modified location
			manifest->debug_mode_path_adjustment = debug_mode_path_adjustment;
#endif

			group_t group;
			group.manifest = manifest;
			group.manifest->set_rid_start(total_asset_count_);

			if(archive_path != nullptr) {
				group.archive = new archive_t();
				if(!group.archive->load(archive_path, total_asset_count_)) {
					delete group.archive;
					console_t::error("Failed to mount archive: %s", archive_path);
					return false;
				}
				console_t::info("Mounted archive: %s", archive_path);
			}
			
			total_asset_count_ += group.manifest->length();
			groups_.push_back(group);

			if(archive_path == nullptr) {
#if _DEBUG
				console_t::info("Mounted loose archive rooted at: %s", (debug_mode_path_adjustment + manifest->common_path_prefix).c_str());
#else
				console_t::info("Mounted loose archive rooted at: %s", manifest->common_path_prefix_.c_str());
#endif
			}
			return true;
		}

		rid_t asset_library_t::path_to_rid(const char *path) const {
			for(const auto &group : groups_) {
				rid_t rid = group.manifest->path_to_rid(path);
				if(rid != RID_NONE)
					return rid;
			}
			return RID_NONE;
		}

		const char *asset_library_t::get_path(rid_t rid) const {
			if(rid == RID_NONE)
				return nullptr;

			for(const auto &group : groups_) {
				if(group.manifest->contains(rid)) {
					if(group.archive != nullptr)
						return group.archive->get_path(rid);
					return group.manifest->get_path(rid);
				}
			}
			return nullptr;
		}

		rid_t asset_library_t::full_path_to_rid(const char *path) const {
			for(const auto &group : groups_) {
				const std::string &prefix = group.manifest->common_path_prefix;
				if(strstr(path, prefix.c_str()) == path) {
					rid_t rid = group.manifest->path_to_rid(path + prefix.length());
					if(rid != RID_NONE)
						return rid;
				}
			}
			return RID_NONE;
		}

		array_t<char> asset_library_t::alloc_read(rid_t rid) const {
			if(rid == RID_NONE)
				return nullptr;

			for(const auto &group : groups_) {
				if(group.manifest->contains(rid)) {
					if(group.archive != nullptr)
						return group.archive->alloc_read(rid);
					return group.manifest->alloc_read(rid);
				}
			}
			return nullptr;
		}

	} // namespace core
} // namespace maki
