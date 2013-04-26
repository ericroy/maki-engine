#include "core/core_stdafx.h"
#include "core/MakiTexture.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace Maki
{

	Texture::Texture() : Resource(), type(TextureType_Regular), width(0), height(0), handle(-1) {
	}
	
	Texture::~Texture() {
		Engine::Get()->renderer->DeleteTexture(this);
	}

	
} // namespace Maki