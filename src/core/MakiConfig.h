#pragma once
#include <memory>
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"

namespace maki {
	namespace core {
	
		class asset_library_t;
		class document_t;

		class config_t
		{
			MAKI_NO_COPY(config_t);

		public:
			config_t(asset_library_t *lib, rid_t rid);

			const char *get_string(const char *key_path, const char *default_value) const;
			int64_t get_int(const char *key_path, int64_t default_value) const;
			uint64_t get_uint(const char *key_path, uint64_t default_value) const;
			bool get_bool(const char *key_path, bool default_value) const;
			float get_float(const char *key_path, float default_value) const;

		private:
			std::unique_ptr<document_t> doc_;
		};

	} // namespace core
} // namespace maki
