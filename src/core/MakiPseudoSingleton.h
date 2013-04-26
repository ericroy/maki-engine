#pragma once
#include "core/core_stdafx.h"

namespace Maki
{

	template<class SubClass>
	class PseudoSingleton
	{
	public:
		static inline SubClass *Get() { return current; }
		static inline SubClass *SetCurrent(SubClass *newCurrent)
		{
			SubClass *oldCurrent = current;
			current = newCurrent;
			return oldCurrent;
		}

	private:
		static SubClass *current;

	public:
		PseudoSingleton()
		{
			if(current == nullptr) {
				current = static_cast<SubClass *>(this);
			}
		}

		virtual ~PseudoSingleton()
		{
			if(current == static_cast<SubClass *>(this)) {
				current = nullptr;
			}
		}
	};

	template<class SubClass>
	SubClass *PseudoSingleton<SubClass>::current = nullptr;

} // namespace Maki