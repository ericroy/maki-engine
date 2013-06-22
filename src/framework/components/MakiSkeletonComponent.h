#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class DebugArmature;

	class SkeletonComponent : public Component
	{
	public:
		static const Type COMPONENT_TYPE = Type::Type_Skeleton;
		static std::function<Entity *()> CreateDebugArmature;

	public:
		SkeletonComponent();
		SkeletonComponent(HandleOrRid skeletonId);
		virtual ~SkeletonComponent();

		virtual bool Init(Document::Node *node);
		void Update(Material *mat);
		inline void SetPoseDirty(bool dirty) { poseDirty = dirty; }

	private:
		bool Init(HandleOrRid skelId);

	public:
		Handle skeleton;
		DebugArmature *armature;
		Array<Skeleton::Joint> pose;

	private:
		Array<Matrix44> matrixPose;
		int32 materialSlot;
		bool poseDirty;
	};


} // namespace Maki