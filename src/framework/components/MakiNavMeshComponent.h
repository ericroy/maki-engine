#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class NavMesh : public Component
			{
			public:
				static const Type TYPE = Type_NavMesh;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				NavMesh();
				virtual ~NavMesh();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid meshId);

			public:
				Handle mesh;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki