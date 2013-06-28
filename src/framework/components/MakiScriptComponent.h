#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Script : public Component
			{
			public:
				static const Type TYPE = Type_Script;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Script();
				Script(HandleOrRid scriptId);
				virtual ~Script();
				bool Init(Document::Node *props);

			private:
				Handle script;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki