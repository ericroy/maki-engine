#include "core/core_stdafx.h"
#include "core/MakiTextureSetManager.h"

namespace Maki
{
	namespace Core
	{
		
		inline bool TextureSetManager::Predicate::operator()(const TextureSet *ts) const
		{
			if(count != ts->textureCount) {
				return false;
			}
			for(uint8 i = 0; i < count; i++) {
				if(textureRids[i] != ts->textureRids[i]) {
					return false;
				}
			}
			return true;
		}
	
		TextureSetManager::TextureSetManager(uint32 size)
			: Manager<TextureSet, TextureSetManager>(size, "TextureSetManager")
		{
		}
	
		TextureSetManager::~TextureSetManager()
		{
		}
	
		Handle TextureSetManager::Load(uint8 count, Rid *textureRids)
		{
			Predicate p;
			p.count = count;
			p.textureRids = textureRids;
			Handle handle = resPool->Match(p) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			TextureSet *ts = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(ts) TextureSet();
			if(!ts->Load(count, textureRids)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}


	} // namespace Core

} // namespace Maki