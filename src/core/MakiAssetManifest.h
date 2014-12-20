#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class asset_manifest_t
		{
		public:
			asset_manifest_t(uint32 asset_count, rid_t *rid_array, const char **path_array, const char *common_path_prefix);
			virtual ~asset_manifest_t();

			inline uint32 get_count() const { return count_; }
			inline bool contains(rid_t rid) const { return (uint32)rid >= rid_start_ && (uint32)rid < rid_start_ + count_; }
			inline const char *get_path(rid_t rid) const { return path_array_[rid - rid_start_]; }
			void set_rid_start(uint32 rid_start);
			rid_t path_to_rid(const char *path) const;
			char *alloc_read(rid_t rid, uint32 *bytesRead = nullptr) const;

		public:
			std::string common_path_prefix_;
			std::string debug_mode_path_adjustment_;

		private:
			uint32 count_;
			uint32 rid_start_;
			rid_t *rid_array_;
			const char **path_array_;
		};

	} // namespace core

} // namespace maki
