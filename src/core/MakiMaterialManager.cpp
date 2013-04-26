#include "core/core_stdafx.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiEngine.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace Maki
{

	MaterialManager::MaterialManager(uint32 size)
		: Manager<Material, MaterialManager>(size)
	{
		assert(size <= (1<<DrawCommand::BITS_PER_MATERIAL) && "MaterialManager too large, add more bits in DrawCommand");
	}
	
	MaterialManager::~MaterialManager()
	{
	}

	Handle MaterialManager::Load(Rid rid)
	{
		if(rid == RID_NONE) {
			return HANDLE_NONE;
		}

		Handle handle = resPool->Match(Resource::FindPredicate<Material>(rid)) | managerId;
		if(handle != HANDLE_NONE) {
			return handle;
		}

		handle = resPool->Alloc() | managerId;
		Material *mat = resPool->Get(handle & HANDLE_VALUE_MASK);
		new(mat) Material();
		if(!mat->Load(rid)) {
			resPool->Free(handle & HANDLE_VALUE_MASK);
			return HANDLE_NONE;
		}
		return handle;
	}

	Handle MaterialManager::DuplicateIfShared(Handle handle)
	{
		MaterialManager *owner = GetOwner(handle);
		if(handle != HANDLE_NONE && owner->resPool->GetRefCount(handle & HANDLE_VALUE_MASK) > 1) {
			// Allocate a new item, relying on the item's copy constructor to duplicate it
			Handle newHandle = owner->resPool->Alloc(*owner->resPool->Get(handle)) | owner->managerId;
			owner->resPool->Free(handle & HANDLE_VALUE_MASK);

			// Must clear the Rid on cloned resources, since they are no longer hot-swappable.
			// Duplicating usually implies an intent to modify the resource, and if you hot-swapped
			// in a new one, those modifications would be lost.
			Material *mat = owner->resPool->Get(newHandle & HANDLE_VALUE_MASK);
			mat->rid = RID_NONE;

			return newHandle;
		}
		return handle;
	}

	void MaterialManager::ReloadAssets()
	{
		const auto end = resPool->End();
		for(auto iter = resPool->Begin(); iter != end; ++iter) {
			Material *mat = iter.Ptr();
			Rid rid = mat->rid;
			if(rid != RID_NONE) {
				Reload(mat);
			}
		}
	}
	
	bool MaterialManager::ReloadAsset(Rid rid)
	{
		if(rid == RID_NONE) {
			return false;
		}
		Handle handle = resPool->Match(Resource::FindPredicate<Material>(rid)) | managerId;
		if(handle == HANDLE_NONE) {
			return false;
		}
		Material *mat = resPool->Get(handle & HANDLE_VALUE_MASK);
		resPool->Free(handle & HANDLE_VALUE_MASK);
		Reload(mat);
		return true;
	}

	void MaterialManager::Reload(Material *mat)
	{
		// Acquire the material's handles while we recreate it
		Handle textureSet = mat->textureSet;
		Handle shaderProgram = mat->shaderProgram;
		TextureSetManager::AddRef(textureSet);
		ShaderProgramManager::AddRef(shaderProgram);

		// Recreate material
		Rid rid = mat->rid;
		mat->~Material();
		new(mat) Material();
		mat->Load(rid);

		// Free the handles that it just loaded up, use the old ones that it had before
		TextureSetManager::Free(mat->textureSet);
		ShaderProgramManager::Free(mat->shaderProgram);
		mat->textureSet = textureSet;
		mat->shaderProgram = shaderProgram;
	}


} // namespace Maki