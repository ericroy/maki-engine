#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{
		
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API std::basic_string<char, std::char_traits<char>, std::allocator<char> >;

		class MAKI_CORE_API Archive
		{
		private:
			enum Flag
			{
				Flag_ZLibCompressed = 1<<0,
			};

			struct MAKI_CORE_API Entry
			{
				uint64 offset;
				uint32 compressedLength;
				uint32 uncompressedLength;
				std::string fileName;
				uint32 flags;
			};

			static const uint32 DECOMPRESSION_BUFFER_SIZE = 32 * 1024;
			static char buffer[DECOMPRESSION_BUFFER_SIZE];

		public:
			Archive();
			Archive(const MoveToken<Archive> &other);
			virtual ~Archive();

			inline bool IsLoaded() const { return fp != nullptr; }
			inline void SetRidStart(uint32 ridStart) { this->ridStart = ridStart; }
			inline bool Contains(Rid rid) const { return (uint32)rid >= ridStart && (uint32)rid < ridStart + entries.count; }
			bool Load(const char *archivePath, uint32 ridStart = 0);
			char *AllocRead(const char *path, uint32 *bytesRead = nullptr) const;
			char *AllocRead(Rid rid, uint32 *bytesRead = nullptr) const;

		private:
			Archive(const Archive &other) {}

		private:
			FILE *fp;
			uint32 ridStart;
			uint32 bodyOffset;
#pragma warning(disable:4251)
			Array<Entry> entries;
#pragma warning(default:4251)
		};

		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Array<Archive::Entry>;

	} // namespace Core

} // namespace Maki