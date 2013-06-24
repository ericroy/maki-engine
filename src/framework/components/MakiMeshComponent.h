#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{

		class DebugArmature;

		namespace Components
		{

			class Mesh : public Component
			{
				friend class RenderSystem;

			public:
				static const Component::Type TYPE = Type_Mesh;

			public:
				Mesh();
				virtual ~Mesh();
				bool Init(Document::Node *props);
				bool Init(HandleOrRid meshId, HandleOrRid matId);
				
				void SetMeshScale(float scale);

			private:
				

			public:
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