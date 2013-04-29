#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMeshEntity.h"

namespace Maki
{
	class BillboardEntity : public Entity
	{
	public:
		enum BillboardMode
		{
			BillboardMode_None = 0,
			BillboardMode_Face,
			BillboardMode_Pivot,
		};

	public:
		BillboardEntity(HandleOrRid matId, HandleOrRid texId);
		BillboardEntity(HandleOrRid matId, HandleOrRid texId, BillboardMode mode, const Vector4 &facingAxis, const Vector4 &constraintAxis);
		virtual ~BillboardEntity();
		void Draw(Renderer *renderer);

	protected:
		bool Init(HandleOrRid matId, HandleOrRid texId);

	private:
		Handle mesh;
		Handle material;
		DrawCommand dc;
		BillboardMode mode;
		Vector4 facingAxis;
		Vector4 constraintAxis;
	};


	class BillboardEntityFactory : private EntityFactory
	{
	public:
		BillboardEntityFactory();
		virtual ~BillboardEntityFactory();
		virtual bool PreCreate(Document::Node *node);
		BillboardEntity *Create();
		virtual void PostCreate(BillboardEntity *e);

	protected:
		Rid matRid;
		Rid textureRid;
		BillboardEntity::BillboardMode mode;
		Vector4 facing;
		Vector4 constraint;
	};

} // namespace Maki