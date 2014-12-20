#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		class texture_set_t
		{
		public:
			static const int32 max_textures_per_set_ = 4;

		public:
			texture_set_t();
			texture_set_t(const move_token_t<texture_set_t> &other);
			texture_set_t(const texture_set_t &) { assert(false && "texture_set_t copy construction not allowed"); }
			~texture_set_t();
			bool load(uint8 count, rid_t *texture_rids);
			inline bool operator==(const texture_set_t &other) const;
			
		public:
			uint8 texture_count_;
			handle_t textures_[max_textures_per_set_];
			rid_t texture_rids_[max_textures_per_set_];
		};


		bool texture_set_t::operator==(const texture_set_t &other) const
		{
			if(texture_count_ != other.texture_count_) {
				return false;
			}
			for(uint32 i = 0; i < texture_count_; i++) {
				if(texture_rids_[i] != other.texture_rids_[i] || texture_rids_[i] == RID_NONE) {
					return false;
				}
			}
			return true;
		}

	} // namespace core

} // namespace maki
