#include "core/MakiMaterialManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace maki {
	namespace core {

		material_manager_t::material_manager_t(uint32_t capacity) {
			res_pool_.reset(new resouce_pool_t<material_t>(capacity, "material_manager_t"));
		}

		ref_t<material_t> material_manager_t::create() {
			return res_pool_->alloc();
		}

		ref_t<material_t> material_manager_t::get(rid_t rid) {
			return res_pool_->find([rid](const material_t &m) {
				return rid == m.rid();
			});
		}

		ref_t<material_t> material_manager_t::load(rid_t rid) {
			auto mat = res_pool_->alloc();
			return mat->load(rid) ? mat : nullptr;
		}

		ref_t<material_t> material_manager_t::get_or_load(rid_t rid) {
			auto mat = get(rid);
			return mat ? mat : load(rid);
		}

		ref_t<material_t> material_manager_t::clone_if_shared(const ref_t<material_t> &mat) {
			if (mat && mat->ref_count() > 1) {
				// Allocate a new item, relying on the item's copy constructor to duplicate it
				auto clone = res_pool_->alloc(*mat);
				// Must clear the rid_t on cloned resources, since they are no longer hot-swappable.
				// Duplicating usually implies an intent to modify the resource, and if you hot-swapped
				// in a new one, those modifications would be lost.
				clone->rid = RID_NONE;
				return clone;
			}
			return mat;
		}

	} // namespace core
} // namespace maki
