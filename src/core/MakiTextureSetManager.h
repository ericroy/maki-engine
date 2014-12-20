#pragma once
#include "core/core_stdafx.h"
#include "core/MakiTextureSet.h"
#include "core/MakiManager.h"

namespace maki
{
	namespace core
	{
		class texture_set_manager_t;
		

		class texture_set_manager_t : public manager_t<texture_set_t, texture_set_manager_t>
		{
		private:
			struct predicate_t : public std::unary_function<const texture_set_t *, bool>
			{
				inline bool operator()(const texture_set_t *ts) const;
				uint8 count_;
				rid_t *texture_rids_;
			};

		public:
			static const int32 default_size_ = 128;

		public:
			texture_set_manager_t(uint32 size = default_size_);	
			virtual ~texture_set_manager_t();	
			handle_t load(uint8 count, rid_t *texture_rids);
		};


		inline bool texture_set_manager_t::predicate_t::operator()(const texture_set_t *ts) const
		{
			if(count_ != ts->texture_count_) {
				return false;
			}
			for(uint8 i = 0; i < count_; i++) {
				if(texture_rids_[i] != ts->texture_rids_[i]) {
					return false;
				}
			}
			return true;
		}

	} // namespace core

} // namespace maki
