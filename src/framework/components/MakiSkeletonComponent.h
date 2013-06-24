#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		class DebugArmature;

		namespace Components
		{

			class Skeleton : public Component
			{
				friend class SkeletonSystem;

			public:
				static const Component::Type TYPE = Type_Skeleton;
				static std::function<Entity *()> CreateDebugArmature;

			public:
				Skeleton();
				virtual ~Skeleton();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid skeletonRid);
				void OnAttach();

				inline void SetPoseDirty(bool dirty) { poseDirty = dirty; }

			public:
				Handle skeleton;
				DebugArmature *armature;
				Array<Core::Skeleton::Joint> pose;

			private:
				Array<Matrix44> matrixPose;
				int32 materialSlot;
				bool poseDirty;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki