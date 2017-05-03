#include "core/MakiMeshManager.h"
#include "core/MakiDrawCommand.h"

namespace maki {
	namespace core {

		mesh_manager_t::mesh_manager_t(uint32_t capacity) : res_pool_(capacity, "mesh_manager_t") {
			MAKI_ASSERT(capacity <= (1 << draw_command_t::bits_per_mesh) && "assign more bits to mesh in draw command bitfield");
		}
	
		ref_t<mesh_t> mesh_manager_t::create() {
			return res_pool_.alloc();
		}

		ref_t<mesh_t> mesh_manager_t::get(rid_t rid) {
			return res_pool_.find([rid](const mesh_t &m) {
				return rid == m.rid();
			});
		}

		ref_t<mesh_t> mesh_manager_t::load(rid_t rid) {
			auto mesh = res_pool_.alloc();
			return mesh_loader_t::load(rid) ? mesh : nullptr;
		}

		ref_t<mesh_t> mesh_manager_t::get_or_load(rid_t rid) {
			auto mesh = get(rid);
			return mesh ? mesh : load(rid);
		}

	} // namespace core
} // namespace maki
