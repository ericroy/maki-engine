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
				uint64_t offset_;
				uint32_t compressed_length_;
				uint32_t uncompressed_length_;
				std::string file_name_;
				uint32_t flags_;
			};

			static const uint32_t file_path_max_length_ = 260;
			static const uint32_t decompression_buffer_size_ = 32 * 1024;
			static char buffer_[decompression_buffer_size_];

		public:
			archive_t();
			archive_t(const move_token_t<archive_t> &other);
			virtual ~archive_t();

			inline bool is_loaded() const { return fp_ != nullptr; }
			inline void set_rid_start(uint32_t rid_start) { rid_start_ = rid_start; }
			inline bool contains(rid_t rid) const { return (uint32_t)rid >= rid_start_ && (uint32_t)rid < rid_start_ + entries.count_; }
			bool load(const char *archive_path, uint32_t rid_start = 0);
			const char *get_path(rid_t rid) const;
			char *alloc_read(const char *path, uint32_t *bytes_read = nullptr) const;
			char *alloc_read(rid_t rid, uint32_t *bytes_read = nullptr) const;

		private:
			archive_t(const archive_t &other) {}

		private:
			FILE *fp_;
			uint32_t rid_start_;
			uint32_t body_offset_;
			array_t<entry_t> entries;
		};

		

	} // namespace core

} // namespace maki
