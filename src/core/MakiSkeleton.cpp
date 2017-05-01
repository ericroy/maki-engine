#include "core/MakiEngine.h"
#include "core/MakiDocument.h"
#include "core/MakiSkeleton.h"

namespace maki {
	namespace core {

		bool skeleton_t::load(rid_t rid) {
			bones_.free();
		
			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Could not parse file as document <rid %ull>", rid);
				return false;
			}

			bones_.set_length(doc.root().length());
			bones_.zero();

			array_t<joint_t> joints(doc.root().length());

			for(uint64_t i = 0; i < bones_.length(); i++) {
				const auto &n = doc.root()[i];
				auto *b = &bones_[i];
				auto *j = &joints[i];
			
				int64_t parent_index = n.resolve_as_int("#0");
				if(parent_index >= 0) {
					assert(bones_[parent_index].child_count < MAX_CHILDREN_PER_BONE);
					bones_[parent_index].children[bones_[parent_index].child_count++] = b;
				}

				j->offset = vector3_t(n[1].value_as_float(), n[2].value_as_float(), n[3].value_as_float());

				vector3_t euler_angles(
					n[4].value_as_float() * MAKI_DEG_TO_RAD,
					n[5].value_as_float() * MAKI_DEG_TO_RAD,
					n[6].value_as_float() * MAKI_DEG_TO_RAD
				);
				j->rot.from_euler_angles(euler_angles);
			}

			rid_ = rid;

			inverse_bind_pose.free();
			inverse_bind_pose.set_size(joints.length);
			calculate_inverse_bind_pose(joints.data(), inverse_bind_pose_.data());
			return true;
		}

		void skeleton_t::calculate_inverse_bind_pose(joint_t *joint_states, matrix44_t *out) {
			uint64_t index = 0;
			calculate_pos_recursive(index, matrix44_t::identity_, joint_states, out);
			for(uint64_t i = 0; i < bones_.length(); i++)
				matrix44_t::affine_inverse(out[i], out[i]);
		}

		void skeleton_t::calculate_world_pose(joint_t *joint_states, matrix44_t *out) {
			uint64_t index = 0;
			calculate_pos_recursive(index, matrix44_t::identity_, joint_states, out);
		}

		void skeleton_t::calculate_pos_recursive(uint64_t &index, const matrix44_t &current, joint_t *joint_states, matrix44_t *out) {
			matrix44_t rot;
			joint_states[index].rot.to_matrix(rot);
			matrix44_t::translation(joint_states[index].offset, rot);
		
			out[index] = current * rot;
			
			const auto &new_current = out[index];
			const auto *b = &bones_[index];
			index++;
			for(uint64_t i = 0; i < b->child_count; i++)
				calculate_pos_recursive(index, new_current, joint_states, out);
		}

	} // namespace core
} // namespace maki
