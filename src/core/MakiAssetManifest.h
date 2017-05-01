#pragma once
#include <string>
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class asset_manifest_t {
			MAKI_NO_COPY(asset_manifest_t);
			
		public:
			asset_manifest_t() = delete;
			asset_manifest_t(uint64_t asset_count, rid_t *rid_array, const char **path_array, const char *common_path_prefix);
			virtual ~asset_manifest_t() = default;

			inline uint64_t length() const { return length_; }
			inline bool contains(rid_t rid) const { return (uint64_t)rid >= rid_start_ && (uint64_t)rid < rid_start_ + length_; }
			inline const char *get_path(rid_t rid) const { return path_array_[rid - rid_start_]; }
			void set_rid_start(uint64_t rid_start);
			rid_t path_to_rid(const char *path) const;
			const char *rid_to_path(rid_t rid) const;
			char *alloc_read(rid_t rid, uint64_t *bytes_read = nullptr) const;

		public:
			std::string common_path_prefix;
			std::string debug_mode_path_adjustment;

		private:
			uint64_t length_ = 0;
			uint64_t rid_start_ = 0;
			rid_t *rid_array_ = nullptr;
			const char **path_array_ = nullptr;
		};

	} // namespace core
} // namespace maki
