#include "core/MakiShaderProgramManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiDrawCommand.h"

namespace maki {
	namespace core {

		shader_program_manager_t::shader_program_manager_t(uint32_t capacity) : res_pool_(capacity, "shader_program_manager_t") {
			MAKI_ASSERT(capacity <= (1 << draw_command_t::bits_per_shader_program) && "assign more bits to shader program in draw command bitfield");
		}

		ref_t<shader_program_t> shader_program_manager_t::get(rid_t rid, shader_program_variant_t variant) {
			return res_pool_->find([rid, variant](const shader_program_t &sp) {
				return rid == sp.rid() && variant == sp.variant();
			});
		}

		ref_t<shader_program_t> shader_program_manager_t::load(rid_t rid, shader_program_variant_t variant) {
			auto sp = res_pool_->alloc();
			if (!sp->load(rid, variant))
				return nullptr;
			auto *eng = engine_t::get();
			if (!eng->renderer->create_shader_program(sp.ptr()))
				return nullptr;
			return sp;
		}

		ref_t<shader_program_t> shader_program_manager_t::get_or_load(rid_t rid, shader_program_variant_t variant) {
			auto ref = get(rid, variant);
			return ref ? ref : load(rid, variant);
		}


	} // namespace core
} // namespace maki
