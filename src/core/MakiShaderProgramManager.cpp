#include "core/MakiShaderProgramManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace maki {
	namespace core {

		shader_program_manager_t::shader_program_manager_t(uint32_t capacity) {
			res_pool_.reset(new resource_pool_t<shader_program_t>(capacity));
		}

		ref_t<shader_program_t> shader_program_manager_t::get(rid_t rid, shader_program_t::variant_t variant) {
			return res_pool_->find([rid, variant](const shader_program_t &res) { return rid == res.rid() && variant == res.variant; });
		}

		ref_t<shader_program_t> shader_program_manager_t::load(rid_t rid, shader_program_t::variant_t variant) {
			auto sh = res_pool_->alloc();
			if (!sh->load(rid, variant))
				return nullptr;
			auto *eng = engine_t::get();
			if (!eng->renderer->create_shader_program(sh.ptr()))
				return nullptr;
			return sh;
		}

		ref_t<shader_program_t> shader_program_manager_t::get_or_load(rid_t rid, shader_program_t::variant_t variant) {
			auto ref = get(rid, variant);
			return ref ? ref : load(rid, variant);
		}


	} // namespace core
} // namespace maki
