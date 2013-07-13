#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace Maki
{
	namespace Core
	{

		TextureSet::TextureSet() : textureCount(0)
		{
			for(uint8 i = 0; i < MAX_TEXTURES_PER_SET; i++) {
				textures[i] = HANDLE_NONE;
				textureRids[i] = RID_NONE;
			}
		}
	
		TextureSet::TextureSet(const MoveToken<TextureSet> &other)
			: textureCount(other.obj->textureCount)
		{
			memcpy(textures, other.obj->textures, sizeof(textures));
			memcpy(textureRids, other.obj->textureRids, sizeof(textureRids));

			for(uint8 i = 0; i < MAX_TEXTURES_PER_SET; i++) {
				other.obj->textures[i] = HANDLE_NONE;
				other.obj->textureRids[i] = RID_NONE;
			}
			other.obj->textureCount = 0;
		}

		TextureSet::~TextureSet()
		{
			TextureManager::Free(textureCount, textures);
		}

		bool TextureSet::Load(uint8 count, Rid *textureRids)
		{
			CoreManagers *res = CoreManagers::Get();

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

	} // namespace Core

} // namespace Maki