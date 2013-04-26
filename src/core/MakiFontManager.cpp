#include "core/core_stdafx.h"
#include "core/MakiFontManager.h"

namespace Maki
{

	inline bool FontManager::Predicate::operator()(const Font *font) const
	{
		return font->rid == fontRid && font->pixelSize == pixelSize && font->shaderProgramRid == shaderProgramRid;
	}

	FontManager::FontManager(uint32 size)
		: Manager<Font, FontManager>(size)
	{
	}
	
	FontManager::~FontManager()
	{
	}

	Handle FontManager::Load(Rid shaderProgramRid, Rid fontRid, uint32 pixelSize)
	{
		
		Predicate p;
		p.fontRid = fontRid;
		p.shaderProgramRid = shaderProgramRid;
		p.pixelSize = pixelSize;
		Handle handle = resPool->Match(p);
		
		if(handle != HANDLE_NONE) {
			return handle;
		}

		handle = resPool->Alloc();
		assert(handle != HANDLE_NONE && "Font pool depleted");
		Font *font = resPool->Get(handle);
		new(font) Font();
		if(!font->Load(shaderProgramRid, fontRid, pixelSize)) {
			resPool->Free(handle);
			return HANDLE_NONE;
		}
		return handle;
	}

} // namespace Maki