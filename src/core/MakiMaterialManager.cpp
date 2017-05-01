#include "core/MakiMaterialManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace maki {
	namespace core {

		material_manager_t::material_manager_t(uint64_t capacity)
			: manager_t<material_t, material_manager_t>(capacity, "material_manager_t") {
			assert(capacity <= (1 << draw_command_t::bits_per_material_) && "material_manager_t too large, add more bits in draw_command_t");
		}

		handle_t material_manager_t::load(rid_t rid) {
			if(rid == RID_NONE)
				return HANDLE_NONE;

			handle_t handle = res_pool_->match(resource_t::find_predicate_t<material_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE)
				return handle;

			handle = res_pool_->alloc() | manager_id_;
			material_t *mat = res_pool_->get(handle & handle_value_mask_);
			new(mat) material_t();
			if(!mat->load(rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		handle_t material_manager_t::duplicate_if_shared(handle_t handle) {
			material_manager_t *owner = get_owner(handle);
			if(handle != HANDLE_NONE && owner->res_pool_->get_ref_count(handle & handle_value_mask_) > 1) {
				// Allocate a new item, relying on the item's copy constructor to duplicate it
				handle_t new_handle = owner->res_pool_->alloc(*owner->res_pool_->get(handle)) | owner->manager_id_;
				owner->res_pool_->free(handle & handle_value_mask_);

				// Must clear the rid_t on cloned resources, since they are no longer hot-swappable.
				// Duplicating usually implies an intent to modify the resource, and if you hot-swapped
				// in a new one, those modifications would be lost.
				material_t *mat = owner->res_pool_->get(new_handle & handle_value_mask_);
				mat->rid = RID_NONE;

				return new_handle;
			}
			return handle;
		}

		void material_manager_t::reload_assets() {
			for(auto iter = std::begin(res_pool_); iter != std::end(res_pool_); ++iter) {
				material_t *mat = iter.ptr();
				rid_t rid = mat->rid;
				if(rid != RID_NONE)
					reload(mat);
			}
		}
	
		bool material_manager_t::reload_asset(rid_t rid) {
			if(rid == RID_NONE)
				return false;
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<material_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE)
				return false;
			material_t *mat = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);
			reload(mat);
			return true;
		}

		void material_manager_t::reload(material_t *mat) {
			// Acquire the material's handles while we recreate it
			handle_t texture_set = mat->texture_set;
			handle_t shader_program = mat->shader_program;
			texture_set_manager_t::add_ref(texture_set);
			shader_program_manager_t::add_ref(shader_program);

			// Recreate material
			rid_t rid = mat->rid;
			mat->~material_t();
			new(mat) material_t();
			mat->load(rid);

			// free the handles that it just loaded up, use the old ones that it had before
			texture_set_manager_t::free(mat->texture_set);
			shader_program_manager_t::free(mat->shader_program);
			mat->texture_set = texture_set;
			mat->shader_program = shader_program;
		}


	} // namespace core
} // namespace maki
