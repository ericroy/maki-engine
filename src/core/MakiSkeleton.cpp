#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiDocument.h"
#include "core/MakiSkeleton.h"

namespace maki
{
	namespace core
	{

		skeleton_t::skeleton_t() : resource_t() {}
		skeleton_t::~skeleton_t() {}

		bool skeleton_t::load(rid_t rid) {
			bones_.free();
		
			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Could not parse file as document <rid %d>", rid);
				return false;
			}

			bones_.set_size(doc.root_->count_);
			bones_.zero();

			array_t<joint_t> joints(doc.root_->count_);

			for(uint32 i = 0; i < bones_.count_; i++) {
				document_t::node_t *n = doc.root_->children_[i];
				Bone *b = &bones_[i];
				joint_t *j = &joints[i];
			
				int32 parent_index = n->resolve_as_int("#0");
				if(parent_index >= 0) {
					assert(bones_[parent_index].child_count_ < MAX_CHILDREN_PER_BONE);
					bones_[parent_index].children_[bones_[parent_index].child_count_++] = b;
				}

				j->offset_ = vector3_t(n->children_[1]->value_as_float(), n->children_[2]->value_as_float(), n->children_[3]->value_as_float());

				vector3_t euler_angles(
					n->children_[4]->value_as_float() * MAKI_DEG_TO_RAD,
					n->children_[5]->value_as_float() * MAKI_DEG_TO_RAD,
					n->children_[6]->value_as_float() * MAKI_DEG_TO_RAD
				);
				j->rot_.from_euler_angles(euler_angles);
			}

			this->rid_ = rid;

			inverse_bind_pose.free();
			inverse_bind_pose.set_size(joints.count_);
			calculate_inverse_bind_pose(joints.data_, inverse_bind_pose.data_);

			return true;
		}

		void skeleton_t::calculate_inverse_bind_pose(joint_t *joint_states, matrix44_t *out) {
			uint32 index = 0;
			calculate_pos_recursive(index, matrix44_t::identity_, joint_states, out);
			for(uint32 i = 0; i < bones_.count_; i++) {
				matrix44_t::affine_inverse(out[i], out[i]);
			}
		}

		void skeleton_t::calculate_world_pose(joint_t *joint_states, matrix44_t *out) {
			uint32 index = 0;
			calculate_pos_recursive(index, matrix44_t::identity_, joint_states, out);
		}

		void skeleton_t::calculate_pos_recursive(uint32 &index, const matrix44_t &current, joint_t *joint_states, matrix44_t *out) {
			matrix44_t rot;
			joint_states[index].rot_.to_matrix(rot);
			matrix44_t::translation(joint_states[index].offset_, rot);
		
			out[index] = current * rot;
			const matrix44_t &new_current = out[index];

			const Bone *b = &bones_[index];
			index++;
			for(uint32 i = 0; i < b->child_count_; i++) {
				calculate_pos_recursive(index, new_current, joint_states, out);
			}
		}

	} // namespace core

} // namespace maki
