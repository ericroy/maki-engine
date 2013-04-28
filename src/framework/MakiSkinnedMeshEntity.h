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
		SkinnedMeshEntity();
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



	class SkinnedMeshEntityFactory : private MeshEntityFactory
	{
	public:
		SkinnedMeshEntityFactory();
		virtual ~SkinnedMeshEntityFactory();
		virtual bool PreCreate(Document::Node *node);
		SkinnedMeshEntity *Create();
		virtual void PostCreate(SkinnedMeshEntity *e);

	protected:
		Rid skelRid;
	};

} // namespace Maki