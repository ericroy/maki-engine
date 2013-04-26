#include "framework/framework_stdafx.h"
#include "framework/MakiMeshEntity.h"

namespace Maki
{

	MeshEntity::MeshEntity()
		: Entity(), mesh(HANDLE_NONE), material(HANDLE_NONE), scaleMatrix(true), meshScale(1.0f)
	{
	}

	MeshEntity::MeshEntity(HandleOrRid meshId, HandleOrRid matId)
		: Entity(), mesh(HANDLE_NONE), material(HANDLE_NONE), scaleMatrix(true), meshScale(1.0f)
	{
		Init(meshId, matId);
	}

	MeshEntity::~MeshEntity() {
		MeshManager::Free(mesh);
		MaterialManager::Free(material);
	}

	void MeshEntity::Init(HandleOrRid meshId, HandleOrRid matId)
	{
		auto res = ResourceProvider::Get();
		
		if(matId.isHandle) {
			if(matId.handle != HANDLE_NONE) {
				MaterialManager::AddRef(matId.handle);
				material = matId.handle;
			}
		} else {
			material = res->materialManager->Load(matId.rid);
		}

		if(meshId.isHandle) {
			assert(meshId.handle != HANDLE_NONE);
			MeshManager::AddRef(meshId.handle);
			mesh = meshId.handle;
		} else {
			mesh = res->meshManager->Load(meshId.rid);
			assert(mesh != HANDLE_NONE);
		}
		
		// Allocate and initialize draw commands
		const Mesh *m = MeshManager::Get(mesh);
		bounds.Merge(BoundingBox(m->bounds.pos, m->bounds.radii));

		const uint32 count = m->siblings.size()+1;
		for(uint32 i = 0; i < count; i++) {
			DrawCommand dc;
			dc.SetMesh(i == 0 ? mesh : m->siblings[i-1]);
			dc.SetMaterial(material);
			drawCommands.push_back(std::move(dc));
		}
	}
	
	void MeshEntity::Draw(Renderer *renderer) {
		Engine *eng = Engine::Get();

		const uint32 count = drawCommands.size();
		for(uint32 i = 0; i < count; i++) {
#if _DEBUG
			// I don't want to set this every draw call for efficiency reasons, but if we don't
			// then hot swapping materials doesn't have any effect.  Perhaps we'll just leave this on
			// in debug mode for now
			drawCommands[i].SetMaterial(material);
#endif
			renderer->Draw(drawCommands[i], GetWorldMatrix() * scaleMatrix);
		}
	}

	void MeshEntity::SetMeshScale(float scale)
	{
		scaleMatrix.SetIdentity();
		Matrix44::Scale(scale, scale, scale, scaleMatrix);
		bounds.radii *= scale / meshScale;
		meshScale = scale;
	}


} // namespace Maki