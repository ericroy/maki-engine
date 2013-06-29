#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Framework
	{

		class Script : public Resource
		{
		public:
			Script();
			virtual ~Script();
			bool Load(Rid scriptRid);

		private:
			Script(const Script &other) {}

		public:
			char *data;
		};

	} // namespace Core

} // namespace Maki