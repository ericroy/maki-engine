#include "core/core_stdafx.h"
#include "core/MakiTextureManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiAssetLibrary.h"

namespace Maki
{
	namespace Core
	{

		TextureManager::TextureManager(uint32 size)
			: Manager<Texture, TextureManager>(size, "TextureManager")
		{
		}
	
		TextureManager::~TextureManager()
		{
		}
	
		Handle TextureManager::AllocTexture(Texture::TextureType type, uint32 width, uint32 height, uint8 channels)
		{
			Engine *eng = Engine::Get();

			Handle handle = resPool->Alloc() | managerId;
			Texture *tex = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(tex) Texture();
			tex->type = type;
			tex->width = width;
			tex->height = height;

			switch(tex->type) {
			case Texture::TextureType_Regular:
				if(eng->renderer->CreateEmptyTexture(tex, channels)) {
					return handle;
				}
				break;
			case Texture::TextureType_DepthStencil:
				if(eng->renderer->CreateDepthTexture(tex)) {
					return handle;
				}
				break;
			case Texture::TextureType_RenderTarget:
				if(eng->renderer->CreateRenderTarget(tex)) {
					return handle;
				}
				break;
			default:
				assert(false);
				break;
			}

			Console::Error("Failed to alloc empty texture");
			resPool->Free(handle & HANDLE_VALUE_MASK);
			return HANDLE_NONE;
		}

		Handle TextureManager::Load(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Texture>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			Texture *tex = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(tex) Texture();
		
			if(!LoadData(tex, rid)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}

		bool TextureManager::LoadData(Texture *tex, Rid rid)
		{
			Engine *eng = Engine::Get();

			uint32 bytesRead;
			char *data = eng->assets->AllocRead(rid, &bytesRead);

			if(!eng->renderer->CreateTexture(tex, (char *)data, bytesRead)) {
				Console::Error("Failed to create texture <rid %d>", rid);
				SAFE_FREE(data);
				return false;
			}

			SAFE_FREE(data);
			tex->rid = rid;
			return true;
		}

		void TextureManager::ReloadAssets()
		{
			const ResourcePool<Texture>::Iterator end = resPool->End();
			for(ResourcePool<Texture>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				Texture *tex = iter.Ptr();
				Rid rid = tex->rid;
				if(rid != RID_NONE) {
					tex->~Texture();
					new(tex) Texture();
					LoadData(tex, rid);
				}
			}
		}

		bool TextureManager::ReloadAsset(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Texture>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			Texture *tex = resPool->Get(handle & HANDLE_VALUE_MASK);
			resPool->Free(handle & HANDLE_VALUE_MASK);

			if(rid != RID_NONE) {
				tex->~Texture();
				new(tex) Texture();
				LoadData(tex, rid);
			}
			return true;
		}


	} // namespace Core

} // namespace Maki