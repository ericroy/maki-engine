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
		bool ret = Init(meshId, matId);
		assert(ret);
	}

	MeshEntity::~MeshEntity()
	{
		const uint32 count = drawCommands.count;
		for(uint32 i = 0; i < count; i++) {
			drawCommands[i].~DrawCommand();
		}

		MeshManager::Free(mesh);
		MaterialManager::Free(material);
	}

	bool MeshEntity::Init(HandleOrRid meshId, HandleOrRid matId)
	{
		ResourceProvider *res = ResourceProvider::Get();
		
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
		drawCommands.SetSize(count);
		for(uint32 i = 0; i < count; i++) {
			DrawCommand *dc = &drawCommands[i];
			new(dc) DrawCommand();
			dc->SetMesh(i == 0 ? mesh : m->siblings[i-1]);
			dc->SetMaterial(material);
		}

		return true;
	}
	
	void MeshEntity::Draw(Renderer *renderer) {
		Engine *eng = Engine::Get();

		const uint32 count = drawCommands.count;
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










	MeshEntityFactory::MeshEntityFactory()
		: EntityFactory(), meshRid(RID_NONE), matRid(RID_NONE)
	{
	}

	MeshEntityFactory::~MeshEntityFactory()
	{
	}

	bool MeshEntityFactory::PreCreate(Document::Node *node)
	{
		if(!EntityFactory::PreCreate(node)) {
			return false;
		}

		Engine *eng = Engine::Get();

		const char *meshPath = node->ResolveValue("mesh.#0");
		if(meshPath == nullptr) {
			Console::Error("Entity did not specify a mesh");
			return false;
		}
		meshRid = eng->assets->PathToRid(meshPath);
		if(meshRid == RID_NONE) {
			Console::Error("No RID for path: %s", meshPath);
			return false;
		}

		const char *matPath = node->ResolveValue("material.#0");
		if(matPath == nullptr) {
			Console::Error("Entity did not specify a material");
			return false;
		}
		matRid = eng->assets->PathToRid(matPath);
		if(matRid == RID_NONE) {
			Console::Error("No RID for path: %s", matRid);
			return false;
		}

		return true;
	}

	MeshEntity *MeshEntityFactory::Create()
	{
		return new MeshEntity(meshRid, matRid);
	}
		
	void MeshEntityFactory::PostCreate(MeshEntity *e)
	{
		EntityFactory::PostCreate(e);
	}



} // namespace Maki