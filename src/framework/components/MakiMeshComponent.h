#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiRenderableComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Mesh : public RenderableComponent
			{
			public:
				static const Type TYPE = Type_Mesh;
				static const TypeFlag DEPENDENCIES = 0;

			public:
				Mesh();
				virtual ~Mesh();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid meshId, HandleOrRid matId);
				bool InitSprite(HandleOrRid matId, HandleOrRid texId, const Rect &rect);
				
				Mesh *Clone(bool prototype);

			public:
				Handle mesh;
				Handle material;
				Array<DrawCommand> drawCommands;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki