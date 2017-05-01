#pragma once
#include "core/MakiMacros.h"
#include "core/MakiManager.h"
#include "core/MakiShaderProgram.h"

namespace maki {
	namespace core {

		class shader_program_manager_t : public manager_t<shader_program_t, shader_program_manager_t> {
			MAKI_NO_COPY(shader_program_manager_t);
		
		public:
			struct find_predicate_t : public std::unary_function<const shader_program_t *, bool> {
				find_predicate_t(rid_t rid, shader_program_t::variant_t variant) : rid(rid), variant(variant) {}
				inline bool operator()(const shader_program_t *res) const {
					return res->rid == rid && res->variant == variant;
				}
				rid_t rid = RID_NONE;
				shader_program_t::variant_t variant = shader_program_t::variant_normal;
			};

		public:
			shader_program_manager_t(uint64_t capacity);
			virtual ~shader_program_manager_t() = default;
			handle_t load(rid_t rid, shader_program_t::variant_t variant = shader_program_t::variant_normal);
			void reload_assets();
			bool reload_asset(rid_t rid);
		};

	} // namespace core
} // namespace maki
