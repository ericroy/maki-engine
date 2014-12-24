#include "core/core_stdafx.h"
#include "core/MakiMeshManager.h"
#include "core/MakiDrawCommand.h"

namespace maki
{
	namespace core
	{

		mesh_manager_t::mesh_manager_t(uint32 size)
			: manager_t<mesh_t, mesh_manager_t>(size, "mesh_manager_t")
		{
			assert(size <= (1<<draw_command_t::bits_per_mesh_) && "mesh_manager_t too large, add more bits in draw_command_t");
		}
	
		mesh_manager_t::~mesh_manager_t()
		{
		}
	
		handle_t mesh_manager_t::load(rid_t rid)
		{
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<mesh_t>(rid)) | manager_id_;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = res_pool_->alloc() | manager_id_;
			mesh_t *mesh = res_pool_->get(handle & handle_value_mask_);
			new(mesh) mesh_t();
			if(!mesh->load(rid)) {
				res_pool_->free(handle & handle_value_mask_);
				return HANDLE_NONE;
			}
			return handle;
		}

		void mesh_manager_t::reload_assets()
		{
			const resource_pool_t<mesh_t>::iterator_t end = res_pool_->end();
			for(resource_pool_t<mesh_t>::iterator_t iter = res_pool_->begin(); iter != end; ++iter) {
				mesh_t *mesh = iter.ptr();
				rid_t rid = mesh->rid_;
				if(rid != RID_NONE) {
					mesh->~mesh_t();
					new(mesh) mesh_t();
					mesh->load(rid);
				}
			}
		}

		bool mesh_manager_t::reload_asset(rid_t rid)
		{
			handle_t handle = res_pool_->match(resource_t::find_predicate_t<mesh_t>(rid)) | manager_id_;
			if(handle == HANDLE_NONE) {
				return false;
			}
			mesh_t *mesh = res_pool_->get(handle & handle_value_mask_);
			res_pool_->free(handle & handle_value_mask_);

			if(rid != RID_NONE) {
				mesh->~mesh_t();
				new(mesh) mesh_t();
				mesh->load(rid);
			}
			return true;
		}

	} // namespace core

} // namespace maki
