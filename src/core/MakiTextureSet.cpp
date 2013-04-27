#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiTextureManager.h"


namespace Maki
{

	TextureSet::TextureSet() : textureCount(0)
	{
		memset(textures, (uint32)HANDLE_NONE, sizeof(textures));
		memset(textureRids, (uint32)RID_NONE, sizeof(textureRids));
	}
	
	TextureSet::TextureSet(const MoveToken<TextureSet> &other)
		: textureCount(other.obj->textureCount)
	{
		memcpy(textures, other.obj->textures, sizeof(textures));
		memcpy(textureRids, other.obj->textureRids, sizeof(textureRids));

		memset(other.obj->textures, (uint32)HANDLE_NONE, sizeof(textures));
		memset(other.obj->textureRids, (uint32)RID_NONE, sizeof(textureRids));
		other.obj->textureCount = 0;
	}

	TextureSet::~TextureSet()
	{
		TextureManager::Free(textureCount, textures);
	}

	bool TextureSet::Load(uint8 count, Rid *textureRids)
	{
		ResourceProvider *res = ResourceProvider::Get();

		textureCount = count;
		for(uint8 i = 0; i < count; i++) {
			this->textureRids[i] = textureRids[i];
			this->textures[i] = res->textureManager->Load(textureRids[i]);
			if(this->textures[i] == HANDLE_NONE) {
				goto failed;
			}
		}
		return true;

failed:
		TextureManager::Free(textureCount, textures);
		return false;
	}

} // namespace Maki