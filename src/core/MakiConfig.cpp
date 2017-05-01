#include "core/MakiConfig.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiDocument.h"
#include "core/MakiConsole.h"

namespace maki {
	namespace core {
	
		config_t::config_t(asset_library_t *lib, rid_t rid)
		{
			doc_.reset(new document_t());

			uint64_t bytes;
			char *data = lib->alloc_read(rid, &bytes);
			if(data == nullptr) {
				console_t::error("Failed to open config file");
				MAKI_SAFE_FREE(data);
				return;
			}
			if(!doc_->load(data, bytes)) {
				console_t::error("Failed to parse config file");
				MAKI_SAFE_FREE(data);
				return;
			}

			MAKI_SAFE_FREE(data);
		}

		const char *config_t::get_string(const char *key_path, const char *default_value) const
		{
			const auto *n = doc_->root().resolve(key_path);
			if(n == nullptr || n->length() == 0)
				return default_value;
			return n[0].value();
		}

		int64_t config_t::get_int(const char *key_path, int64_t default_value) const
		{
			const auto *n = doc_->root().resolve(key_path);
			if(n == nullptr || n->length() == 0)
				return default_value;
			int64_t out;
			if(!n[0].value_as_int(&out)) {
				console_t::warning("config_t value '%s' was not an int", key_path);
				return default_value;
			}
			return out;
		}

		uint64_t config_t::get_uint(const char *key_path, uint64_t default_value) const
		{
			const auto *n = doc_->root().resolve(key_path);
			if(n == nullptr || n->length() == 0)
				return default_value;
			uint64_t out;
			if(!n[0].value_as_uint(&out)) {
				console_t::warning("config_t value '%s' was not a uint", key_path);
				return default_value;
			}
			return out;
		}

		bool config_t::get_bool(const char *key_path, bool default_value) const
		{
			const auto *n = doc_->root().resolve(key_path);
			if(n == nullptr || n->length() == 0)
				return default_value;
			bool out;
			if(!n[0].value_as_bool(&out)) {
				console_t::warning("config_t value '%s' was not a bool", key_path);
				return default_value;
			}
			return out;
		}

		float config_t::get_float(const char *key_path, float default_value) const
		{
			const auto *n = doc_->root().resolve(key_path);
			if(n == nullptr || n->length() == 0)
				return default_value;
			float out;
			if(!n[0].value_as_float(&out)) {
				console_t::warning("config_t value '%s' was not a float", key_path);
				return default_value;
			}
			return out;
		}

	} // namespace core
} // namespace maki
