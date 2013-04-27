#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	template<class T>
	class MoveToken
	{
	public:
		template<class U>
		inline operator MoveToken<U>()
		{
			assert(dynamic_cast<U *>(obj) != nullptr);
			return {(U *)obj};
		}

	public:
		T *obj;
	};

	template<class T>
	inline MoveToken<T> Move(T &obj) { MoveToken<T> m; m.obj = &obj; return m; }

} // namespace Maki