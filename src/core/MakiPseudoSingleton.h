#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		template<class SUBCLASS>
		class pseudo_singleton_t
		{
		public:
			static inline SUBCLASS *get() { return current_; }
			static inline SUBCLASS *set_current(SUBCLASS *new_current)
			{
				SUBCLASS *old_current = current_;
				current_ = new_current;
				return old_current;
			}

		private:
			static SUBCLASS *current_;

		public:
			pseudo_singleton_t()
			{
				if(current_ == nullptr) {
					current_ = static_cast<SUBCLASS *>(this);
				}
			}

			virtual ~pseudo_singleton_t()
			{
				if(current_ == static_cast<SUBCLASS *>(this)) {
					current_ = nullptr;
				}
			}
		};

		template<class SUBCLASS>
		SUBCLASS *pseudo_singleton_t<SUBCLASS>::current_ = nullptr;

	} // namespace core

} // namespace maki
