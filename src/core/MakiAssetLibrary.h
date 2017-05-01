#pragma once
#include <vector>
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"

namespace maki {
	namespace core {
	
		class asset_manifest_t;
		class archive_t;

		class asset_library_t {
			MAKI_NO_COPY(asset_library_t);

		private:
			struct group_t
			{
				asset_manifest_t *manifest = nullptr;
				archive_t *archive = nullptr;
			};

		public:
			asset_library_t() = default;
			virtual ~asset_library_t();

			bool mount(asset_manifest_t *manifest, const char *archive_path = nullptr);
			rid_t path_to_rid(const char *path) const;
			rid_t full_path_to_rid(const char *path) const;
			const char *get_path(rid_t rid) const;
			char *alloc_read(rid_t rid, uint64_t *bytes_read = nullptr) const;

		public:
			// Allow a path prefix to be applied for all assets, but only in debug mode
			std::string debug_mode_path_adjustment;

		private:
			uint64_t total_asset_count_ = 0;
			std::vector<group_t> groups_;
		};

		

	} // namespace core
} // namespace maki
