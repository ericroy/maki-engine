#pragma once
#include "core/MakiArray.h"
#include "core/MakiMatrix44.h"

namespace maki {
	namespace core {

		class skeleton_t {
			MAKI_NO_COPY(skeleton_t);

		public:
			static const int64_t MAX_BONES = 128;
			static const int64_t MAX_CHILDREN_PER_BONE = 7;

			struct __declspec(align(MAKI_SIMD_ALIGN)) joint_t : public aligned_t<MAKI_SIMD_ALIGN> {
				quaternion_t rot;
				vector3_t offset;
			};

			struct __declspec(align(MAKI_SIMD_ALIGN)) bone_t : public aligned_t<MAKI_SIMD_ALIGN> {
				bone_t *children[MAX_CHILDREN_PER_BONE];
				uint64_t child_count;
			};
	
		public:
			skeleton_t() = default;
			bool load(rid_t rid);
			void calculate_world_pose(joint_t *joint_states, matrix44_t *out);
			inline const array_t<matrix44_t> &inverse_bind_pose() const { return inverse_bind_pose_; }
			inline const array_t<bone_t> &bones() const { return bones_; }
	
		private:
			void calculate_inverse_bind_pose(joint_t *joint_states, matrix44_t *out);
			void calculate_pos_recursive(uint64_t &index, const matrix44_t &current, joint_t *joint_states, matrix44_t *out);

		public:
			rid_t rid = RID_NONE;

		private:
			array_t<matrix44_t> inverse_bind_pose_;
			array_t<bone_t> bones_;
		};

	} // namespace core
} // namespace maki
