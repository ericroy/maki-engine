#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureManager.h"


namespace maki
{
	namespace core
	{

		texture_set_t::texture_set_t() : texture_count_(0)
		{
			for(uint8 i = 0; i < max_textures_per_set_; i++) {
				textures_[i] = HANDLE_NONE;
				texture_rids_[i] = RID_NONE;
			}
		}
	
		texture_set_t::texture_set_t(const move_token_t<texture_set_t> &other)
			: texture_count_(other.obj_->texture_count_)
		{
			memcpy(textures_, other.obj_->textures_, sizeof(textures_));
			memcpy(texture_rids_, other.obj_->texture_rids_, sizeof(texture_rids_));

			for(uint8 i = 0; i < max_textures_per_set_; i++) {
				other.obj_->textures_[i] = HANDLE_NONE;
				other.obj_->texture_rids_[i] = RID_NONE;
			}
			other.obj_->texture_count_ = 0;
		}

		texture_set_t::~texture_set_t()
		{
			texture_manager_t::free(texture_count_, textures_);
		}

		bool texture_set_t::load(uint8 count, rid_t *texture_rids_)
		{
			core_managers_t *res = core_managers_t::get();

			texture_count_ = count;
			for(uint8 i = 0; i < count; i++) {
				this->texture_rids_[i] = texture_rids_[i];
				this->textures_[i] = res->texture_manager_->load(texture_rids_[i]);
				if(this->textures_[i] == HANDLE_NONE) {
					goto failed;
				}
			}
			return true;

		failed:
			texture_manager_t::free(texture_count_, textures_);
			return false;
		}

	} // namespace core

} // namespace maki
