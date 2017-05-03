#pragma once
#include "core/MakiTypes.h"
#include "core/MakiManager.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiResourcePool.h"

namespace maki {
	namespace core {

		class shader_program_manager_t {
			MAKI_NO_COPY(shader_program_manager_t);
		public:
			shader_program_manager_t(uint32_t capacity);
			virtual ~shader_program_manager_t() = default;
			ref_t<shader_program_t> get(rid_t rid, shader_program_variant_t variant = shader_program_variant_normal);
			ref_t<shader_program_t> load(rid_t rid, shader_program_variant_t variant = shader_program_variant_normal);
			ref_t<shader_program_t> get_or_load(rid_t rid, shader_program_variant_t variant = shader_program_variant_normal);
		private:
			resource_pool_t<shader_program_t> res_pool_;
		};

	} // namespace core
} // namespace maki
