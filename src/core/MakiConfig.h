#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
	
		class asset_library_t;
		class document_t;

		class config_t
		{
		public:
			config_t(asset_library_t *lib, rid_t rid);
			virtual ~config_t();

			const char *get_string(const char *key_path, const char *default_value) const;
			int32 get_int(const char *key_path, int32 default_value) const;
			uint32 get_uint(const char *key_path, uint32 default_value) const;
			bool get_bool(const char *key_path, bool default_value) const;
			float get_float(const char *key_path, float default_value) const;

		private:
			document_t *doc_;
		};

	} // namespace core

} // namespace maki
