#pragma once
#include "core/MakiTypes.h"
#include "core/MakiShader.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		enum shader_program_variant_t : uint8_t {
			shader_program_variant_normal = 0,
			shader_program_variant_depth,
			shader_program_variant_shadow,
			shader_program_variant_max = shader_program_variant_shadow
		};

		namespace {
			const char *variant_data_key[shader_program_variant_max + 1];
			const char *variant_meta_key[shader_program_variant_max + 1];
		}

		class shader_program_t {
			MAKI_NO_COPY(shader_program_t);
		public:
			shader_program_t() = default;
			~shader_program_t();

			inline rid_t rid() const { return rid_; }
			inline void set_rid(rid_t rid) { rid_ = rid; }
			inline shader_program_variant_t variant() const { return variant_; }
			inline shader_t &vertex_shader() { return vertex_shader_; }
			inline const shader_t &vertex_shader() const { return vertex_shader_; }
			inline shader_t &fragment_shader() { return fragment_shader_; }
			inline const shader_t &fragment_shader() const { return fragment_shader_; }
			inline uint32_t input_attribute_count() const { return input_attribute_count_; }
			inline uintptr_t handle() const { return handle_; }
			inline void set_handle(uintptr_t handle) { handle_ = handle; }
			inline const ref_t<shader_program_t> &get_variant(shader_program_variant_t variant) {
				MAKI_ASSERT(variant != shader_program_variant_normal);
				return variants_[(uint32_t)variant - 1];
			}

			bool load(rid_t rid, shader_program_variant_t variant = shader_program_variant_normal);

		private:
			rid_t rid_ = RID_NONE;
			uint32_t input_attribute_count_ = 0;
			shader_t vertex_shader_;
			shader_t fragment_shader_;
			shader_program_variant_t variant_ = shader_program_variant_normal;
			ref_t<shader_program_t> variants_[shader_program_variant_max];
			uintptr_t handle_ = 0;
		};

	} // namespace core
} // namespace maki
