#pragma once
#include "core/MakiTypes.h"
#include "core/MakiArray.h"

namespace maki {
	namespace core {

		class asset_manifest_t {
			MAKI_NO_COPY(asset_manifest_t);
			
		public:
			asset_manifest_t() = delete;
			asset_manifest_t(uint32_t asset_count, rid_t *rid_array, const char **path_array, const char *common_path_prefix);
			virtual ~asset_manifest_t() = default;

			inline uint32_t length() const {
				return length_;
			}

			inline bool contains(rid_t rid) const {
				return (uint32_t)rid >= rid_start_ && (uint32_t)rid < rid_start_ + length_;
			}

			inline const char *get_path(rid_t rid) const {
				return path_array_[rid - rid_start_];
			}

			void set_rid_start(uint32_t rid_start);
			rid_t path_to_rid(const char *path) const;
			array_t<char> alloc_read(rid_t rid) const;

		public:
			string common_path_prefix;
			string debug_mode_path_adjustment;

		private:
			uint32_t length_ = 0;
			uint32_t rid_start_ = 0;
			rid_t *rid_array_ = nullptr;
			const char **path_array_ = nullptr;
		};

	} // namespace core
} // namespace maki
