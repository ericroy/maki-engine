#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	template<class T>
	class Movable
	{
	public:
		template<class U>
		inline operator Movable<U>()
		{
			assert(dynamic_cast<U *>(obj) != nullptr && "Cannot cast a movable to this type");
			return {(U *)obj};
		}

	public:
		T *obj;
	};

	template<class T>
	inline Movable<T> Move(T &obj) { Movable<T> m; m.obj = &obj; return m; }

} // namespace Maki