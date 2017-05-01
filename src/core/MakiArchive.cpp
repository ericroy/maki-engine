#include <cstdio>
#include <algorithm>
#include "core/MakiArchive.h"
#include "core/MakiConsole.h"
#include "dependencies/miniz.h"

#if MAKI_OS_WIN
#	define MAKI_FSEEK64 _fseeki64
#else
#	define MAKI_FSEEK64 fseeko64
#endif

using namespace std;

namespace maki {
	namespace core {

		char archive_t::buffer_[archive_t::decompression_buffer_size_];

		archive_t::archive_t(archive_t &&other)
			: fp_(other.fp_),
			rid_start_(other.rid_start_),
			body_offset_(other.body_offset_),
			entries_(std::move(other.entries_))
		{
			other.fp_ = nullptr;
			other.rid_start_ = 0;
			other.body_offset_ = 0;
		}

		archive_t::~archive_t() {
			if(fp_ != nullptr)
				fclose(fp_);
		}

		bool archive_t::load(const char *archive_path, uint64_t rid_start) {
			rid_start_ = rid_start;

			// Already initialized
			if(fp_ != nullptr)
				return false;
			
			fp_ = fopen(archive_path, "rb");
			if(fp_ == nullptr)
				return false;

			fread(&body_offset_, sizeof(body_offset_), 1, fp_);
			
			uint64_t toc_count = 0;
			fread(&toc_count, sizeof(toc_count), 1, fp_);
			entries_.set_length(toc_count);
			entries_.zero();
			
			// Read table of contents
			char buffer_[file_path_max_length_];
			uint64_t toc_offset = ftell(fp_);

			for (auto &entry : entries_) {
				fread(&entry.flags, sizeof(entry.flags), 1, fp_);

				uint32_t name_length = 0;
				fread(&name_length, sizeof(name_length), 1, fp_);
				assert(name_length < file_path_max_length_);

				fread(buffer_, sizeof(char), name_length, fp_);
				entry.file_name = std::string(buffer_, name_length);

				while(ftell(fp_) % 8 != 0) { fseek(fp_, 1, SEEK_CUR); }

				fread(&entry.offset, sizeof(entry.offset), 1, fp_);
				fread(&entry.uncompressed_length, sizeof(entry.uncompressed_length), 1, fp_);
				fread(&entry.compressed_length, sizeof(entry.compressed_length), 1, fp_);
			}
			return true;
		}

		const char *archive_t::get_path(rid_t rid) const {
			assert(fp_ != nullptr);
			uint64_t i = (uint64_t)rid - rid_start_;
			return entries_[i].file_name.c_str();
		}

		array_t<char> archive_t::alloc_read(const char *path) const {
			assert(fp_ != nullptr);
			for(uint32_t i = 0; i < entries_.length(); i++) {
				if(entries_[i].file_name == path) {
					uint64_t rid = rid_start_ + i;
					return alloc_read(MAKI_TO_RID(rid));
				}
			}
			console_t::error("Failed to find resource in archive: %s", path);
			return nullptr;
		}

		array_t<char> archive_t::alloc_read(rid_t rid) const {
			assert(fp_ != nullptr);
			uint64_t i = (uint64_t)rid - rid_start_;

			auto dest = array_t<char>(entries_[i].uncompressed_length + 1);
			dest[entries_[i].uncompressed_length] = 0;

			if((entries_[i].flags & flag_zlib_compressed) != 0) {
				MAKI_FSEEK64(fp_, body_offset_ + entries_[i].offset, SEEK_SET);

				z_stream stream;
				memset(&stream, 0, sizeof(stream));
				stream.next_out = (uint8_t *)dest.data();
				stream.avail_out = entries_[i].uncompressed_length;
				inflateInit(&stream);
				
				int32_t status = Z_OK;
				uint64_t remaining = entries_[i].compressed_length;
				while(remaining > 0) {
					stream.next_in = (const uint8_t *)buffer_;
					stream.avail_in = (uint32_t)fread(buffer_, sizeof(char), (uint32_t)std::min(decompression_buffer_size_, remaining), fp_);
					status = inflate(&stream, Z_SYNC_FLUSH);
					stream.next_out = (uint8_t *)(dest.data() + stream.total_out);
					stream.avail_out = (uint32_t)(entries_[i].uncompressed_length - stream.total_out);
					remaining = entries_[i].compressed_length - stream.total_in;
				}
				assert(status == Z_STREAM_END || status == Z_OK);
				status = inflateEnd(&stream);
				assert(status == Z_OK);
			} else {
				MAKI_FSEEK64(fp_, body_offset_ + entries_[i].offset, SEEK_SET);
				uint64_t read = fread(dest.data(), sizeof(char), entries_[i].uncompressed_length, fp_);
				assert(read == entries_[i].uncompressed_length);
			}

			return dest;
		}

	} // namespace core
} // namespace maki
