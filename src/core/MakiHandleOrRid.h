#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API HandleOrRid
		{
		public:
			inline HandleOrRid() : isHandle(true), handle(HANDLE_NONE) {}
			inline HandleOrRid(Handle h) : isHandle(true), handle(h) {}
			inline HandleOrRid(Rid r) : isHandle(false), rid(r) {}

		public:
			bool isHandle;
			union
			{
				Handle handle;
				Rid rid;
			};
		};

	} // namespace Core

} // namespace Maki