#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class DebugArmature;

	class MeshComponent : public Component
	{
		friend class RenderSystem;

	public:
		static const Type COMPONENT_TYPE = Type::Type_Mesh;
		static std::function<Entity *()> CreateDebugArmature;

	public:
		MeshComponent();
		MeshComponent(HandleOrRid meshId, HandleOrRid materialId);
		MeshComponent(HandleOrRid meshId, HandleOrRid materialId, HandleOrRid skeletonId);
		virtual ~MeshComponent();

		virtual bool Init(Document::Node *node);
		void Draw(Renderer *renderer, const Matrix44 &world);
		void Update(float dt);
		void SetMeshScale(float scale);
		inline void SetPoseDirty(bool dirty) { poseDirty = dirty; }

	private:
		bool Init(HandleOrRid meshId, HandleOrRid matId, HandleOrRid skelId);

	public:
		Handle mesh;
		Handle material;
		Array<DrawCommand> drawCommands;
		Entity *drawListNext;

		// For skinned meshes only
		Handle skeleton;
		DebugArmature *armature;
		Array<Skeleton::Joint> pose;

	private:
		float meshScale;
		Matrix44 scaleMatrix;

		// For skinned meshes only
		Array<Matrix44> matrixPose;
		int32 materialSlot;
		bool poseDirty;
	};


} // namespace Maki