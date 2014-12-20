#include "core/core_stdafx.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace maki
{
	namespace core
	{

		shader_program_manager_t::shader_program_manager_t(uint32 size)
			: manager_t<shader_program_t, shader_program_manager_t>(size, "shader_program_manager_t")
		{
			assert(size <= (1<<draw_command_t::bits_per_shader_program_) && "shader_program_manager_t too large, add more bits in draw_command_t");
		}
	
		shader_program_manager_t::~shader_program_manager_t()
		{
		}

		handle_t shader_program_manager_t::load(rid_t rid, shader_program_t::variant_t variant)
		{
			handle_t handle = resPool->Match(find_predicate_t(rid, variant)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->alloc() | managerId;
			shader_program_t *shader = resPool->get(handle & handle_value_mask_);
			new(shader) shader_program_t();
		
			if(!shader->load(rid, variant)) {
				resPool->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}

			engine_t *eng = engine_t::get();
			if(!eng->renderer_->create_shader_program(shader)) {
				resPool->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}

			return handle;
		}

		void shader_program_manager_t::reload_assets()
		{
			engine_t *eng = engine_t::get();
			const resource_pool_t<shader_program_t>::iterator_t end = resPool->end();
			for(resource_pool_t<shader_program_t>::iterator_t iter = resPool->begin(); iter != end; ++iter) {
				shader_program_t *shader = iter.Ptr();
				rid_t rid = shader->rid_;
				shader_program_t::variant_t variant = shader->variant_;

				if(rid != RID_NONE) {
					shader->~shader_program_t();
					new(shader) shader_program_t();
					if(shader->load(rid, variant)) {
						eng->renderer_->create_shader_program(shader);
					}
				}
			}
		}
	
		bool shader_program_manager_t::reload_asset(rid_t rid)
		{
			bool found = false;
			for(uint32 variant = shader_program_t::variant_normal_; variant < shader_program_t::variant_count_; variant++) {
				handle_t handle = resPool->Match(find_predicate_t(rid, (shader_program_t::variant_t)variant)) | managerId;
				if(handle == HANDLE_NONE) {
					continue;
				}
				found = true;

				shader_program_t *shader = resPool->get(handle & handle_value_mask_);
				resPool->free(handle & handle_value_mask_);

				if(rid != RID_NONE) {
					shader->~shader_program_t();
					new(shader) shader_program_t();
					if(shader->load(rid, (shader_program_t::variant_t)variant)) {
						engine_t::get()->renderer_->create_shader_program(shader);
					}
				}
			}
			return found;
		}

	} // namespace core

} // namespace maki
