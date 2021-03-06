#include "core/core_stdafx.h"
#include "core/MakiArchive.h"
#include "dependencies/miniz.h"

namespace Maki
{
	namespace Core
	{

		char Archive::buffer[Archive::DECOMPRESSION_BUFFER_SIZE];

		Archive::Archive()
			: fp(nullptr), ridStart(0), bodyOffset(0)
		{
		}

		Archive::Archive(const MoveToken<Archive> &other)
			: fp(other.obj->fp), ridStart(other.obj->ridStart), bodyOffset(other.obj->bodyOffset)
		{
			std::swap(entries, other.obj->entries);
			other.obj->fp = nullptr;
			other.obj->ridStart = 0;
			other.obj->bodyOffset = 0;
		}

		Archive::~Archive()
		{
			if(fp != nullptr) {
				fclose(fp);
			}
		}

		bool Archive::Load(const char *archivePath, uint32 ridStart)
		{
			this->ridStart = ridStart;

			if(fp != nullptr) {
				// Already initialized
				return false;
			}
			
			fp = fopen(archivePath, "rb");
			if(fp == nullptr) {
				return false;
			}

			fread(&bodyOffset, sizeof(bodyOffset), 1, fp);
			
			uint32 tocCount = 0;
			fread(&tocCount, sizeof(tocCount), 1, fp);
			entries.SetSize(tocCount);
			entries.Zero();
			
			// Read table of contents
			char buffer[FILE_PATH_MAX_LENGTH];
			uint32 tocOffset = ftell(fp);
			for(uint32 i = 0; i < tocCount; i++) {
				Entry &entry = entries[i];
				fread(&entry.flags, sizeof(entry.flags), 1, fp);

				uint32 nameLength = 0;
				fread(&nameLength, sizeof(nameLength), 1, fp);
				assert(nameLength < FILE_PATH_MAX_LENGTH);

				fread(buffer, sizeof(char), nameLength, fp);
				entry.fileName = std::string(buffer, nameLength);

				while(ftell(fp) % 8 != 0) { fseek(fp, 1, SEEK_CUR); }

				fread(&entry.offset, sizeof(entry.offset), 1, fp);
				fread(&entry.uncompressedLength, sizeof(entry.uncompressedLength), 1, fp);
				fread(&entry.compressedLength, sizeof(entry.compressedLength), 1, fp);
			}
			return true;
		}

		char *Archive::AllocRead(const char *path, uint32 *bytesRead) const
		{
			assert(fp != nullptr);
			for(uint32 i = 0; i < entries.count; i++) {
				if(entries[i].fileName == path) {
					uint32 rid = ridStart + i;
					return AllocRead(TO_RID(rid), bytesRead);
				}
			}

			Console::Error("Failed to find resource in archive: %s", path);
			return nullptr;
		}

		char *Archive::AllocRead(Rid rid, uint32 *bytesRead) const
		{
			assert(fp != nullptr);
			uint32 i = (uint32)rid - ridStart;

			char *dest = (char *)Allocator::Malloc(entries[i].uncompressedLength+1);
			dest[entries[i].uncompressedLength] = 0;

			if((entries[i].flags & Flag_ZLibCompressed) != 0) {
				MAKI_FSEEK64(fp, bodyOffset + entries[i].offset, SEEK_SET);

				z_stream stream;
				memset(&stream, 0, sizeof(stream));
				stream.next_out = (uint8 *)dest;
				stream.avail_out = entries[i].uncompressedLength;
				inflateInit(&stream);
				
				int32 status = Z_OK;
				uint32 remaining = entries[i].compressedLength;
				while(remaining > 0) {
					stream.next_in = (const uint8 *)buffer;
					stream.avail_in = fread(buffer, sizeof(char), std::min<uint32>(DECOMPRESSION_BUFFER_SIZE, remaining), fp);
					status = inflate(&stream, Z_SYNC_FLUSH);
					stream.next_out = (uint8 *)(dest + stream.total_out);
					stream.avail_out = entries[i].uncompressedLength - stream.total_out;
					remaining = entries[i].compressedLength - stream.total_in;
				}
				assert(status == Z_STREAM_END || status == Z_OK);
				status = inflateEnd(&stream);
				assert(status == Z_OK);
			} else {
				MAKI_FSEEK64(fp, bodyOffset + entries[i].offset, SEEK_SET);
				uint32 read = fread(dest, sizeof(char), entries[i].uncompressedLength, fp);
				assert(read == entries[i].uncompressedLength);
			}

			if(bytesRead != nullptr) {
				*bytesRead = entries[i].uncompressedLength;
			}
			return dest;
		}

	} // namespace Core

} // namespace Maki