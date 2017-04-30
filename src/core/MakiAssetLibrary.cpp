#include "core/core_stdafx.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiAssetManifest.h"
#include "core/MakiArchive.h"

namespace maki
{
	namespace core
	{

		asset_library_t::asset_library_t()
			: total_asset_count_(0)
		{
		}
		
		asset_library_t::~asset_library_t()
		{
			const uint32_t count = groups_.size();
			for(uint32_t i = 0; i < count; i++) {
				MAKI_SAFE_DELETE(groups_[i].archive_);
				// DON'T delete manifest, we do not own it
			}
		}

		bool asset_library_t::mount(asset_manifest_t *manifest, const char *archive_path)
		{
#if _DEBUG
			// Stomp out the archive_ path in debug mode so we will always load loose files from disk
			archive_path = nullptr;

			// In debug mode, the path adjustment is used to make all assets load from a modified location
			manifest->debug_mode_path_adjustment_ = debug_mode_path_adjustment_;
#endif

			group_t group;
			group.manifest_ = manifest;
			group.manifest_->set_rid_start(total_asset_count_);

			if(archive_path != nullptr) {
				group.archive_ = new archive_t();
				if(!group.archive_->load(archive_path, total_asset_count_)) {
					delete group.archive_;
					console_t::error("Failed to mount archive: %s", archive_path);
					return false;
				}
				console_t::info("Mounted archive: %s", archive_path);
			}
			
			total_asset_count_ += group.manifest_->get_count();
			groups_.push_back(group);

			if(archive_path == nullptr) {
#if _DEBUG
				console_t::info("Mounted loose archive rooted at: %s", (debug_mode_path_adjustment_ + manifest->common_path_prefix_).c_str());
#else
				console_t::info("Mounted loose archive rooted at: %s", manifest->common_path_prefix_.c_str());
#endif
			}
			return true;
		}

		rid_t asset_library_t::path_to_rid(const char *path) const
		{
			const uint32_t count = groups_.size();
			for(uint32_t i = 0; i < count; i++) {
				rid_t rid = groups_[i].manifest_->path_to_rid(path);
				if(rid != RID_NONE) {
					return rid;
				}
			}
			return RID_NONE;
		}

		const char *asset_library_t::get_path(rid_t rid) const
		{
			if(rid == RID_NONE) {
				return nullptr;
			}

			const uint32_t count = groups_.size();
			for(uint32_t i = 0; i < count; i++) {
				const group_t &group = groups_[i];
				if(group.manifest_->contains(rid)) {
					if(group.archive_ != nullptr) {
						return group.archive_->get_path(rid);
					}
					return group.manifest_->get_path(rid);
				}
			}
			return nullptr;
		}

		rid_t asset_library_t::full_path_to_rid(const char *path) const
		{
			const uint32_t count = groups_.size();
			for(uint32_t i = 0; i < count; i++) {
				const std::string &prefix = groups_[i].manifest_->common_path_prefix_;
				if(strstr(path, prefix.c_str()) == path) {
					rid_t rid = groups_[i].manifest_->path_to_rid(path + prefix.length());
					if(rid != RID_NONE) {
						return rid;
					}
				}
			}
			return RID_NONE;
		}

		char *asset_library_t::alloc_read(rid_t rid, uint32_t *bytes_read) const
		{
			if(rid == RID_NONE) {
				return nullptr;
			}

			const uint32_t count = groups_.size();
			for(uint32_t i = 0; i < count; i++) {
				const group_t &group = groups_[i];
				if(group.manifest_->contains(rid)) {
					if(group.archive_ != nullptr) {
						return group.archive_->alloc_read(rid, bytes_read);
					}
					return group.manifest_->alloc_read(rid, bytes_read);
				}
			}
			return nullptr;
		}

	} // namespace core

} // namespace maki
