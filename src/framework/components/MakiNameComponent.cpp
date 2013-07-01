#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiNameComponent.h"


namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Name::Name()
				: Component(TYPE, DEPENDENCIES)
			{
				name[0] = 0;
			}

			Name::~Name()
			{
			}

			bool Name::Init(Document::Node *node)
			{
				const char *s = node->ResolveValue("#0");
				assert(strlen(s) < sizeof(name) / sizeof(name[0]));
				strcpy_s(name, s);
				return true;
			}

			bool Name::Init(const char *s)
			{
				assert(strlen(s) < sizeof(name) / sizeof(name[0]));
				strcpy_s(name, s);
				return true;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki