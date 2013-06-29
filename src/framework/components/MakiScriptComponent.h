#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{
			class ScriptingSystem;
		}

		namespace Components
		{

			class Script : public Component
			{
				friend class Systems::ScriptingSystem;

			public:
				static const Type TYPE = Type_Script;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Script();
				virtual ~Script();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid scriptId);

			private:
				Handle script;
			};


		} // namspace Components

	} // namespace Framework

} // namespace Maki