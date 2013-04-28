#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{	
	class Mesh;
	class Renderer;

	class MeshEntity : public Entity
	{
	public:
		MeshEntity();
		MeshEntity(HandleOrRid mesh, HandleOrRid material);
		virtual ~MeshEntity();
		virtual void Draw(Renderer *renderer);
		void SetMeshScale(float scale);

	protected:
		bool Init(HandleOrRid mesh, HandleOrRid material);

	public:
		Handle mesh;
		Handle material;
		Array<DrawCommand> drawCommands;

	private:
		float meshScale;
		Matrix44 scaleMatrix;
	};


	class MeshEntityFactory : private EntityFactory
	{
	public:
		MeshEntityFactory();
		virtual ~MeshEntityFactory();
		virtual bool PreCreate(Document::Node *node);
		MeshEntity *Create();
		virtual void PostCreate(MeshEntity *e);

	protected:
		Rid meshRid;
		Rid matRid;
	};

} // namespace Maki