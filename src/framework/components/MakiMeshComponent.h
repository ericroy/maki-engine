#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{
			class RenderSystem;
		}

		namespace Components
		{

			class Mesh : public Component
			{
				friend class Systems::RenderSystem;

			public:
				enum Flag
				{
					Flag_Visible = 1<<0,
					Flag_CastShadow = 1<<1,
					Flag_Physics = 1<<2,
				};

			public:
				static const uint32 DEFAULT_FLAGS = Flag_Visible;
				static const Component::Type TYPE = Type_Mesh;

			public:
				Mesh();
				virtual ~Mesh();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid meshId, HandleOrRid matId);
				void SetMeshScale(float scale);

			public:
				uint32 flags;
				Handle mesh;
				Handle material;
				Array<DrawCommand> drawCommands;
				Entity *drawListNext;

			private:
				float meshScale;
				Matrix44 scaleMatrix;
				BoundingBox bounds;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki