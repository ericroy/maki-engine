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
			static const int32_t max_constants = 16;

			struct constant_t {
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

			inline rid_t rid() const { return rid_; }
			inline void set_rid(rid_t rid) { rid_ = rid; }

			inline const ref_t<texture_set_t> &texture_set() const { return texture_set_; }
			inline void set_texture_set(const ref_t<texture_set_t> &ts) { texture_set_ = ts; }
			inline void set_texture_set(ref_t<texture_set_t> &&ts) { texture_set_ = forward<ref_t<texture_set_t>>(ts); }

			inline const ref_t<shader_program_t> &shader_program() const { return shader_program_; }
			inline void set_shader_program(const ref_t<shader_program_t> &sp) { shader_program_ = sp; }
			inline void set_shader_program(ref_t<shader_program_t> &&sp) { shader_program_ = forward<ref_t<shader_program_t>>(sp); }

			// Push a constant buffer which will be copied into this object.
			// Returns the index of the constant value in this material.
			// Returns -1 on failure
			int32_t push_constant(const char *key, const array_t<char> &data);

			inline uint8_t constant_count() const { return constant_count_; }
			inline const constant_t &get_constant(uint32_t i) const {
				MAKI_ASSERT(i < constant_count_);
				return constants_[i];
			}
		
		private:
			rid_t rid_ = RID_NONE;
			ref_t<texture_set_t> texture_set_;
			ref_t<shader_program_t> shader_program_;
			uint8_t constant_count_ = 0;
			constant_t constants_[max_constants];
		};

	} // namespace core
} // namespace maki
