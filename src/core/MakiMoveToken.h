#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		template<class T>
		class move_token_t
		{
		public:
			template<class U>
			inline operator move_token_t<U>()
			{
				assert(dynamic_cast<U *>(obj_) != nullptr);
				return {(U *)obj_};
			}

		public:
			T *obj_;
		};

		template<class T>
		inline move_token_t<T> maki_move(T &obj) { move_token_t<T> m; m.obj_ = &obj; return m; }

	} // namespace core

} // namespace maki
