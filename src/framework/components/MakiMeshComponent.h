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
		static const uint64 COMPONENT_TYPE = Type_Mesh;

	public:
		MeshComponent();
		MeshComponent(HandleOrRid meshId, HandleOrRid materialId);
		virtual ~MeshComponent();

		virtual bool Init(Document::Node *node);
		void SetMeshScale(float scale);

	private:
		bool Init(HandleOrRid meshId, HandleOrRid matId);

	public:
		Handle mesh;
		Handle material;
		Array<DrawCommand> drawCommands;
		Entity *drawListNext;

	private:
		float meshScale;
		Matrix44 scaleMatrix;
		BoundingBox bounds;
	};


} // namespace Maki