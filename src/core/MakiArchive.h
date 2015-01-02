#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class archive_t
		{
		private:
			enum flag_t
			{
				flag_zlib_compressed_ = 1<<0,
			};

			struct entry_t
			{
				uint64 offset_;
				uint32 compressed_length_;
				uint32 uncompressed_length_;
				std::string file_name_;
				uint32 flags_;
			};

			static const uint32 file_path_max_length_ = 260;
			static const uint32 decompression_buffer_size_ = 32 * 1024;
			static char buffer_[decompression_buffer_size_];

		public:
			archive_t();
			archive_t(const move_token_t<archive_t> &other);
			virtual ~archive_t();

			inline bool is_loaded() const { return fp_ != nullptr; }
			inline void set_rid_start(uint32 rid_start) { rid_start_ = rid_start; }
			inline bool contains(rid_t rid) const { return (uint32)rid >= rid_start_ && (uint32)rid < rid_start_ + entries.count_; }
			bool load(const char *archive_path, uint32 rid_start = 0);
			const char *get_path(rid_t rid) const;
			char *alloc_read(const char *path, uint32 *bytes_read = nullptr) const;
			char *alloc_read(rid_t rid, uint32 *bytes_read = nullptr) const;

		private:
			archive_t(const archive_t &other) {}

		private:
			FILE *fp_;
			uint32 rid_start_;
			uint32 body_offset_;
			array_t<entry_t> entries;
		};

		

	} // namespace core

} // namespace maki
