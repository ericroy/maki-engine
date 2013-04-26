#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMeshEntity.h"
#include "framework/MakiDebugArmature.h"

namespace Maki
{

	class SkinnedMeshEntity : public MeshEntity
	{
	public:
		static std::function<Entity *()> CreateDebugArmature;

	public:
		SkinnedMeshEntity(HandleOrRid meshId, HandleOrRid matId, HandleOrRid skelId);
		virtual ~SkinnedMeshEntity();

		void Update(float dt);
		inline void SetPoseDirty(bool dirty) { poseDirty = dirty; }

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