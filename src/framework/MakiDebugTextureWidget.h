#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"
#include "framework/MakiMeshEntity.h"


namespace Maki
{

	class DebugTextureWidget : public Entity
	{
	public:
		DebugTextureWidget(Rid materialRid, Handle texture, float size)
			: Entity()
		{
			auto res = ResourceProvider::Get();
			
			Material mat;
			mat.Load(materialRid);

			TextureSet ts;
			ts.textures[ts.textureCount++] = texture;
			TextureManager::AddRef(texture);
			mat.textureSet = res->textureSetManager->Add(std::move(ts));
		
			Handle material = res->materialManager->Add(std::move(mat));
			Handle mesh = res->meshManager->Add(std::move(Mesh(Mesh::PremadeObject_UnitRect)));

			MeshEntity *me = new MeshEntity(mesh, material);
			AddChild(me);

			MaterialManager::Free(material);
			MeshManager::Free(mesh);

			Texture *tex = TextureManager::Get(texture);
			me->SetMeshScale(min(size, tex->width));

			bounds.Merge(me->bounds);
		}
	};

} // namespace Maki