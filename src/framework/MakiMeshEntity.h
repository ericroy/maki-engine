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
		void Draw(Renderer *renderer);
		void SetMeshScale(float scale);

	protected:
		void Init(HandleOrRid mesh, HandleOrRid material);

	public:
		Handle mesh;
		Handle material;
		std::vector<DrawCommand> drawCommands;

	private:
		float meshScale;
		Matrix44 scaleMatrix;
	};


} // namespace Maki