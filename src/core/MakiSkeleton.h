#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace maki
{
	namespace core
	{

		


		class skeleton_t : public resource_t
		{
		public:
			static const int32 MAX_BONES = 128;
			static const int32 MAX_CHILDREN_PER_BONE = 7;

			struct __declspec(align(MAKI_SIMD_ALIGN)) Joint : public aligned_t<MAKI_SIMD_ALIGN>
			{
				quaternion_t rot_;
				vector3_t offset_;
			};

			struct __declspec(align(MAKI_SIMD_ALIGN)) Bone : public aligned_t<MAKI_SIMD_ALIGN>
			{
				Bone *children_[MAX_CHILDREN_PER_BONE];
				uint32 child_count_;
			};
	
		public:
			skeleton_t();
			skeleton_t(const move_token_t<skeleton_t> &) { assert(false && "skeleton_t move construction not allowed"); }
			skeleton_t(const skeleton_t &) { assert(false && "skeleton_t copy construction not allowed"); }
			~skeleton_t();
			bool load(rid_t rid);
			void calculate_world_pose(Joint *joint_states, matrix44_t *out);
	
		private:
			void calculate_inverse_bind_pose(Joint *joint_states, matrix44_t *out);
			void calculate_pos_recursive(uint32 &index, const matrix44_t &current, Joint *joint_states, matrix44_t *out);

		public:
			array_t<matrix44_t> inverse_bind_pose_;

			array_t<Bone> bones_;

		};


		


	} // namespace core

} // namespace maki
