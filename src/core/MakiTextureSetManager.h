#pragma once
#include <functional>
#include "core/MakiTextureSet.h"
#include "core/MakiManager.h"

namespace maki {
	namespace core {

		class texture_set_manager_t : public manager_t<texture_set_t, texture_set_manager_t> {
		private:
			class predicate_t : public std::unary_function<const texture_set_t *, bool> {
			public:
				inline bool operator()(const texture_set_t *ts) const;
				uint8_t count = 0;
				rid_t *texture_rids = nullptr;
			};

		public:
			texture_set_manager_t(uint64_t capacity);
			virtual ~texture_set_manager_t() = default;
			handle_t load(uint8_t count, rid_t *texture_rids);
		};


		inline bool texture_set_manager_t::predicate_t::operator()(const texture_set_t *ts) const
		{
			if(count != ts->texture_count)
				return false;
			for(uint8_t i = 0; i < count; i++) {
				if(texture_rids[i] != ts->texture_rids[i])
					return false;
			}
			return true;
		}

	} // namespace core
} // namespace maki
