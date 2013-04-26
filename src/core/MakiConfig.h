#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	class AssetLibrary;
	class Document;

	class Config
	{
	public:
		Config(AssetLibrary *lib, Rid rid);
		virtual ~Config();

		const std::string GetString(const char *keyPath, const char *defaultValue) const;
		int32 GetInt(const char *keyPath, int32 defaultValue) const;
		uint32 GetUint(const char *keyPath, uint32 defaultValue) const;
		bool GetBool(const char *keyPath, bool defaultValue) const;
		float GetFloat(const char *keyPath, float defaultValue) const;

	private:
		Document *doc;
	};

} // namespace Maki