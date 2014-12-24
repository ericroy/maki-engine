#include "core/core_stdafx.h"
#include "core/MakiArchive.h"
#include "dependencies/miniz.h"

namespace maki
{
	namespace core
	{

		char archive_t::buffer_[archive_t::decompression_buffer_size_];

		archive_t::archive_t()
			: fp_(nullptr), rid_start_(0), body_offset_(0)
		{
		}

		archive_t::archive_t(const move_token_t<archive_t> &other)
			: fp_(other.obj_->fp_), rid_start_(other.obj_->rid_start_), body_offset_(other.obj_->body_offset_)
		{
			std::swap(entries, other.obj_->entries);
			other.obj_->fp_ = nullptr;
			other.obj_->rid_start_ = 0;
			other.obj_->body_offset_ = 0;
		}

		archive_t::~archive_t()
		{
			if(fp_ != nullptr) {
				fclose(fp_);
			}
		}

		bool archive_t::load(const char *archive_path, uint32 rid_start)
		{
			rid_start_ = rid_start;

			if(fp_ != nullptr) {
				// Already initialized
				return false;
			}
			
			fp_ = fopen(archive_path, "rb");
			if(fp_ == nullptr) {
				return false;
			}

			fread(&body_offset_, sizeof(body_offset_), 1, fp_);
			
			uint32 toc_count = 0;
			fread(&toc_count, sizeof(toc_count), 1, fp_);
			entries.set_size(toc_count);
			entries.zero();
			
			// Read table of contents
			char buffer_[file_path_max_length_];
			uint32 toc_offset = ftell(fp_);
			for(uint32 i = 0; i < toc_count; i++) {
				entry_t &entry = entries[i];
				fread(&entry.flags_, sizeof(entry.flags_), 1, fp_);

				uint32 name_length = 0;
				fread(&name_length, sizeof(name_length), 1, fp_);
				assert(name_length < file_path_max_length_);

				fread(buffer_, sizeof(char), name_length, fp_);
				entry.file_name_ = std::string(buffer_, name_length);

				while(ftell(fp_) % 8 != 0) { fseek(fp_, 1, SEEK_CUR); }

				fread(&entry.offset_, sizeof(entry.offset_), 1, fp_);
				fread(&entry.uncompressed_length_, sizeof(entry.uncompressed_length_), 1, fp_);
				fread(&entry.compressed_length_, sizeof(entry.compressed_length_), 1, fp_);
			}
			return true;
		}

		char *archive_t::alloc_read(const char *path, uint32 *bytes_read) const
		{
			assert(fp_ != nullptr);
			for(uint32 i = 0; i < entries.count_; i++) {
				if(entries[i].file_name_ == path) {
					uint32 rid = rid_start_ + i;
					return alloc_read(MAKI_TO_RID(rid), bytes_read);
				}
			}

			console_t::error("Failed to find resource in archive: %s", path);
			return nullptr;
		}

		char *archive_t::alloc_read(rid_t rid, uint32 *bytes_read) const
		{
			assert(fp_ != nullptr);
			uint32 i = (uint32)rid - rid_start_;

			char *dest = (char *)allocator_t::malloc(entries[i].uncompressed_length_+1);
			dest[entries[i].uncompressed_length_] = 0;

			if((entries[i].flags_ & flag_zlib_compressed_) != 0) {
				MAKI_FSEEK64(fp_, body_offset_ + entries[i].offset_, SEEK_SET);

				z_stream stream;
				memset(&stream, 0, sizeof(stream));
				stream.next_out = (uint8 *)dest;
				stream.avail_out = entries[i].uncompressed_length_;
				inflateInit(&stream);
				
				int32 status = Z_OK;
				uint32 remaining = entries[i].compressed_length_;
				while(remaining > 0) {
					stream.next_in = (const uint8 *)buffer_;
					stream.avail_in = fread(buffer_, sizeof(char), std::min<uint32>(decompression_buffer_size_, remaining), fp_);
					status = inflate(&stream, Z_SYNC_FLUSH);
					stream.next_out = (uint8 *)(dest + stream.total_out);
					stream.avail_out = entries[i].uncompressed_length_ - stream.total_out;
					remaining = entries[i].compressed_length_ - stream.total_in;
				}
				assert(status == Z_STREAM_END || status == Z_OK);
				status = inflateEnd(&stream);
				assert(status == Z_OK);
			} else {
				MAKI_FSEEK64(fp_, body_offset_ + entries[i].offset_, SEEK_SET);
				uint32 read = fread(dest, sizeof(char), entries[i].uncompressed_length_, fp_);
				assert(read == entries[i].uncompressed_length_);
			}

			if(bytes_read != nullptr) {
				*bytes_read = entries[i].uncompressed_length_;
			}
			return dest;
		}

	} // namespace core

} // namespace maki
