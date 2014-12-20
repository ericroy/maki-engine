#include "core/core_stdafx.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace maki
{
	namespace core
	{

		material_manager_t::material_manager_t(uint32 size)
			: manager_t<material_t, material_manager_t>(size, "material_manager_t")
		{
			assert(size <= (1<<draw_command_t::bits_per_material_) && "material_manager_t too large, add more bits in draw_command_t");
		}
	
		material_manager_t::~material_manager_t()
		{
		}

		handle_t material_manager_t::load(rid_t rid)
		{
			if(rid == RID_NONE) {
				return HANDLE_NONE;
			}

			handle_t handle = res_pool_->match(resource_t::find_predicate_t<material_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = res_pool_->alloc() | manager_id_;
			material_t *mat = res_pool_->get(handle & handle_value_mask_);
			new(mat) material_t();
			if(!mat->load(rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		handle_t material_manager_t::duplicate_if_shared(handle_t handle)
		{
			material_manager_t *owner = get_owner(handle);
			if(handle != HANDLE_NONE && owner->res_pool_->get_ref_count(handle & handle_value_mask_) > 1) {
				// Allocate a new item, relying on the item's copy constructor to duplicate it
				handle_t new_handle = owner->res_pool_->alloc(*owner->res_pool_->get(handle)) | owner->manager_id_;
				owner->res_pool_->free(handle & handle_value_mask_);

				// Must clear the rid_t on cloned resources, since they are no longer hot-swappable.
				// Duplicating usually implies an intent to modify the resource, and if you hot-swapped
				// in a new one, those modifications would be lost.
				material_t *mat = owner->res_pool_->get(new_handle & handle_value_mask_);
				mat->rid_ = RID_NONE;

				return new_handle;
			}
			return handle;
		}

		void material_manager_t::reload_assets()
		{
			const resource_pool_t<material_t>::iterator_t end = res_pool_->end();
			for(resource_pool_t<material_t>::iterator_t iter = res_pool_->begin(); iter != end; ++iter) {
				material_t *mat = iter.Ptr();
				rid_t rid = mat->rid_;
				if(rid != RID_NONE) {
					reload(mat);
				}
			}
		}
	
		bool material_manager_t::reload_asset(rid_t rid)
		{
			if(rid == RID_NONE) {
				return false;
			}
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<material_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE) {
				return false;
			}
			material_t *mat = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);
			reload(mat);
			return true;
		}

		void material_manager_t::reload(material_t *mat)
		{
			// Acquire the material's handles while we recreate it
			handle_t texture_set = mat->texture_set_;
			handle_t shader_program = mat->shader_program_;
			texture_set_manager_t::add_ref(texture_set);
			shader_program_manager_t::add_ref(shader_program);

			// Recreate material
			rid_t rid = mat->rid_;
			mat->~material_t();
			new(mat) material_t();
			mat->load(rid);

			// free the handles that it just loaded up, use the old ones that it had before
			texture_set_manager_t::free(mat->texture_set_);
			shader_program_manager_t::free(mat->shader_program_);
			mat->texture_set_ = texture_set;
			mat->shader_program_ = shader_program;
		}


	} // namespace core

} // namespace maki
