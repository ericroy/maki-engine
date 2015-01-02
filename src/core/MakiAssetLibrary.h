#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
	
		class asset_manifest_t;
		class archive_t;

		class asset_library_t
		{
		private:
			struct group_t
			{
				group_t() : manifest_(nullptr), archive_(nullptr) {}
				asset_manifest_t *manifest_;
				archive_t *archive_;
			};

		public:
			asset_library_t();
			virtual ~asset_library_t();

			bool mount(asset_manifest_t *manifest, const char *archive_path = nullptr);
			rid_t path_to_rid(const char *path) const;
			rid_t full_path_to_rid(const char *path) const;
			const char *get_path(rid_t rid) const;
			char *alloc_read(rid_t rid, uint32 *bytes_read = nullptr) const;

		public:
			// Allow a path prefix to be applied for all assets, but only in debug mode
			std::string debug_mode_path_adjustment_;

		private:
			uint32 total_asset_count_;
			std::vector<group_t> groups_;
		};

		

	} // namespace core

} // namespace maki
