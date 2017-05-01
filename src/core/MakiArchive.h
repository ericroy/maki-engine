#pragma once
#include <string>
#include "core/MakiMacros.h"
#include "core/MakiArray.h"

namespace maki {
	namespace core {

		class archive_t {
			MAKI_NO_COPY(archive_t);

		private:
			enum flag_t {
				flag_zlib_compressed = 1<<0,
			};

			struct entry_t {
				uint64_t offset;
				uint32_t compressed_length;
				uint32_t uncompressed_length;
				std::string file_name;
				uint32_t flags;
			};

			static const uint32_t file_path_max_length_ = 260;
			static const uint64_t decompression_buffer_size_ = 32 * 1024;
			static char buffer_[decompression_buffer_size_];

		public:
			archive_t() = default;
			archive_t(archive_t &&other);
			virtual ~archive_t();

			inline bool is_loaded() const { return fp_ != nullptr; }
			inline void set_rid_start(uint32_t rid_start) { rid_start_ = rid_start; }
			inline bool contains(rid_t rid) const { return (uint64_t)rid >= rid_start_ && (uint64_t)rid < rid_start_ + entries_.length(); }
			bool load(const char *archive_path, uint64_t rid_start = 0);
			const char *get_path(rid_t rid) const;
			char *alloc_read(const char *path, uint64_t *bytes_read = nullptr) const;
			char *alloc_read(rid_t rid, uint64_t *bytes_read = nullptr) const;

		private:
			FILE *fp_ = nullptr;
			uint64_t rid_start_ = 0;
			uint64_t body_offset_ = 0;
			array_t<entry_t> entries_;
		};

		

	} // namespace core
} // namespace maki
