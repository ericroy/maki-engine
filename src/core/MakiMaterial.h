#pragma once
#include "core/MakiTypes.h"
#include "core/MakiArray.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiTextureSet.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class material_t {
		public:
			static const int32_t max_uniforms = 16;

			struct uniform_value_t {
				// Offset within the shader
				int32_t vs_location = -1;
				int32_t ps_location = -1;
				// Actual data for this constant
				array_t<char> data;
			};

		public:
			material_t() = default;
			explicit material_t(const material_t &other) = default;
			material_t(material_t &&other);
			bool load(rid_t rid);

			// Push a constant buffer which will be copied into this object.
			// Returns the index of the constant value in this material.
			// Returns -1 on failure
			int32_t push_constant(const char *key, const array_t<char> &data);
		
		public:
			rid_t rid = RID_NONE;
			ref_t<texture_set_t> texture_set;
			ref_t<shader_program_t> shader_program;
			uint8_t uniform_count = 0;
			uniform_value_t uniform_values[max_uniforms];
		};

	} // namespace core
} // namespace maki
