#include "core/core_stdafx.h"
#include "core/MakiConfig.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiDocument.h"

namespace Maki
{
	namespace Core
	{
	
		Config::Config(AssetLibrary *lib, Rid rid)
		{
			doc = new Document();

			uint32 bytes;
			char *data = lib->AllocRead(rid, &bytes);
			if(data == nullptr) {
				goto failed;
			}
			if(!doc->Load(data, bytes)) {
				goto failed;
			}

			SAFE_FREE(data);
			return;

		failed:
			Console::Error("Failed to parse config file");
			SAFE_DELETE(doc);
			SAFE_FREE(data);
		}

		Config::~Config()
		{
			SAFE_DELETE(doc);
		}

		const char *Config::GetString(const char *keyPath, const char *defaultValue) const
		{
			Document::Node *n = doc->root->Resolve(keyPath);
			if(n == nullptr || n->count == 0) { return defaultValue; }
			return n->children[0]->value;
		}

		int32 Config::GetInt(const char *keyPath, int32 defaultValue) const
		{
			Document::Node *n = doc->root->Resolve(keyPath);
			if(n == nullptr || n->count == 0) { return defaultValue; }
			int32 out;
			if(!n->children[0]->ValueAsInt(&out)) {
				Console::Warning("Config value '%s' was not an int", keyPath);
				return defaultValue;
			}
			return out;
		}

		uint32 Config::GetUint(const char *keyPath, uint32 defaultValue) const
		{
			Document::Node *n = doc->root->Resolve(keyPath);
			if(n == nullptr || n->count == 0) { return defaultValue; }
			uint32 out;
			if(!n->children[0]->ValueAsUInt(&out)) {
				Console::Warning("Config value '%s' was not a uint", keyPath);
				return defaultValue;
			}
			return out;
		}

		bool Config::GetBool(const char *keyPath, bool defaultValue) const
		{
			Document::Node *n = doc->root->Resolve(keyPath);
			if(n == nullptr || n->count == 0) { return defaultValue; }
			bool out;
			if(!n->children[0]->ValueAsBool(&out)) {
				Console::Warning("Config value '%s' was not a bool", keyPath);
				return defaultValue;
			}
			return out;
		}

		float Config::GetFloat(const char *keyPath, float defaultValue) const
		{
			Document::Node *n = doc->root->Resolve(keyPath);
			if(n == nullptr || n->count == 0) { return defaultValue; }
			float out;
			if(!n->children[0]->ValueAsFloat(&out)) {
				Console::Warning("Config value '%s' was not a float", keyPath);
				return defaultValue;
			}
			return out;
		}

	} // namespace Core

} // namespace Maki