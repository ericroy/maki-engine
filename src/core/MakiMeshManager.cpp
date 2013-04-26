#include "core/core_stdafx.h"
#include "core/MakiMeshManager.h"
#include "core/MakiDrawCommand.h"

namespace Maki
{

	MeshManager::MeshManager(uint32 size)
		: Manager<Mesh, MeshManager>(size)
	{
		assert(size <= (1<<DrawCommand::BITS_PER_MESH) && "MeshManager too large, add more bits in DrawCommand");
	}
	
	MeshManager::~MeshManager()
	{
	}
	
	Handle MeshManager::Load(Rid rid)
	{
		Handle handle = resPool->Match(Resource::FindPredicate<Mesh>(rid)) | managerId;
		if(handle != HANDLE_NONE) {
			return handle;
		}

		handle = resPool->Alloc() | managerId;
		Mesh *mesh = resPool->Get(handle & HANDLE_VALUE_MASK);
		new(mesh) Mesh();
		if(!mesh->Load(rid)) {
			resPool->Free(handle & HANDLE_VALUE_MASK);
			return HANDLE_NONE;
		}
		return handle;
	}

	void MeshManager::ReloadAssets()
	{
		const auto end = resPool->End();
		for(auto iter = resPool->Begin(); iter != end; ++iter) {
			Mesh *mesh = iter.Ptr();
			Rid rid = mesh->rid;
			if(rid != RID_NONE) {
				mesh->~Mesh();
				new(mesh) Mesh();
				mesh->Load(rid);
			}
		}
	}

	bool MeshManager::ReloadAsset(Rid rid)
	{
		Handle handle = resPool->Match(Resource::FindPredicate<Mesh>(rid)) | managerId;
		if(handle == HANDLE_NONE) {
			return false;
		}
		Mesh *mesh = resPool->Get(handle & HANDLE_VALUE_MASK);
		resPool->Free(handle & HANDLE_VALUE_MASK);

		if(rid != RID_NONE) {
			mesh->~Mesh();
			new(mesh) Mesh();
			mesh->Load(rid);
		}
		return true;
	}

} // namespace Maki