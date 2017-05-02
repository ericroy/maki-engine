#pragma once
#include "core/MakiMacros.h"
#include "core/MakiTypes.h"
#include "core/MakiResource.h"
#include "core/MakiShader.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class shader_program_t : public resource_t {
			MAKI_NO_COPY(shader_program_t);
		public:
			enum variant_t {
				variant_normal = 0,
				variant_depth,
				variant_shadow,
				variant_max = variant_shadow
			};

		private:
			static const char *variant_data_key_[variant_max + 1];
			static const char *variant_meta_key_[variant_max + 1];

		public:
			shader_program_t() = default;
			~shader_program_t();
			bool load(rid_t rid, variant_t variant);
			inline bool operator==(const shader_program_t &other) const {
				return rid_ == other.rid_ && variant == other.variant;
			}

		public:
			uint32_t input_attribute_count = 0;
			shader_t vertex_shader;
			shader_t pixel_shader;
			variant_t variant = variant_normal;
			ref_t<shader_program_t> variants[variant_max];
			intptr_t handle = 0;
		};

	} // namespace core
} // namespace maki
