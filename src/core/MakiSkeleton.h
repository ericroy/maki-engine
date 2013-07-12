#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API Skeleton : public Resource
		{
		public:
			static const int32 MAX_BONES = 128;
			static const int32 MAX_CHILDREN_PER_BONE = 7;

			__declspec(align(SIMD_ALIGN)) struct MAKI_CORE_API Joint : public Aligned<SIMD_ALIGN>
			{
				Quaternion rot;
				Vector3 offset;
			};

			__declspec(align(SIMD_ALIGN)) struct MAKI_CORE_API Bone : public Aligned<SIMD_ALIGN>
			{
				Bone *children[MAX_CHILDREN_PER_BONE];
				uint32 childCount;
			};
	
		public:
			Skeleton();
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