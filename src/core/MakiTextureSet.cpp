#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace maki
{
	namespace core
	{

		texture_set_t::texture_set_t() : textureCount(0)
		{
			for(uint8 i = 0; i < MAX_TEXTURES_PER_SET; i++) {
				textures[i] = HANDLE_NONE;
				textureRids[i] = RID_NONE;
			}
		}
	
		texture_set_t::texture_set_t(const move_token_t<texture_set_t> &other)
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

		texture_set_t::~texture_set_t()
		{
			texture_manager_t::free(textureCount, textures);
		}

		bool texture_set_t::load(uint8 count, rid_t *textureRids)
		{
			core_managers_t *res = core_managers_t::get();

			textureCount = count;
			for(uint8 i = 0; i < count; i++) {
				this->textureRids[i] = textureRids[i];
				this->textures[i] = res->texture_manager_->load(textureRids[i]);
				if(this->textures[i] == HANDLE_NONE) {
					goto failed;
				}
			}
			return true;

		failed:
			texture_manager_t::free(textureCount, textures);
			return false;
		}

	} // namespace core

} // namespace maki
