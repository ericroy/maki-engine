#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Name : public Component
			{
			public:
				static const Type TYPE = Type_Name;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Name();
				virtual ~Name();
				bool Init(Document::Node *props);
				bool Init(const char *name);
				Name *Clone(bool prototype);

			public:
				char name[16];
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki