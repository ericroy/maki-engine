#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class AssetManifest
		{
		public:
			AssetManifest(uint32 assetCount, Rid *ridArray, const char **pathArray, const char *commonPathPrefix);
			virtual ~AssetManifest();

			inline uint32 GetCount() const { return count; }
			inline bool Contains(Rid rid) const { return (uint32)rid >= ridStart && (uint32)rid < ridStart + count; }
			inline const char *GetPath(Rid rid) const { return pathArray[rid - ridStart]; }
			void SetRidStart(uint32 ridStart);
			Rid PathToRid(const char *path) const;
			char *AllocRead(Rid rid, uint32 *bytesRead = nullptr) const;

		public:
			std::string commonPathPrefix;
			std::string debugModePathAdjustment;

		private:
			uint32 count;
			uint32 ridStart;
			Rid *ridArray;
			const char **pathArray;
		};

	} // namespace Core

} // namespace Maki