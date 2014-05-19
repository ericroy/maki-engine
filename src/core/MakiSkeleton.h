#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Core
	{

		


		class Skeleton : public Resource
		{
		public:
			static const int32 MAX_BONES = 128;
			static const int32 MAX_CHILDREN_PER_BONE = 7;

			struct __declspec(align(SIMD_ALIGN)) Joint : public Aligned<SIMD_ALIGN>
			{
				Quaternion rot;
				Vector3 offset;
			};

			struct __declspec(align(SIMD_ALIGN)) Bone : public Aligned<SIMD_ALIGN>
			{
				Bone *children[MAX_CHILDREN_PER_BONE];
				uint32 childCount;
			};
	
		public:
			Skeleton();
			Skeleton(const MoveToken<Skeleton> &) { assert(false && "Skeleton move construction not allowed"); }
			Skeleton(const Skeleton &) { assert(false && "Skeleton copy construction not allowed"); }
			~Skeleton();
			bool Load(Rid rid);
			void CalculateWorldPose(Joint *jointStates, Matrix44 *out);
	
		private:
			void CalculateInverseBindPose(Joint *jointStates, Matrix44 *out);
			void CalculatePoseRecursive(uint32 &index, const Matrix44 &current, Joint *jointStates, Matrix44 *out);

		public:
			Array<Matrix44> inverseBindPose;

			Array<Bone> bones;

		};


		


	} // namespace Core

} // namespace Maki