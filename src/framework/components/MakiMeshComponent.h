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
				Mesh(HandleOrRid meshId, HandleOrRid materialId);
				virtual ~Mesh();

				virtual bool Init(Document::Node *node);
				void SetMeshScale(float scale);

			private:
				bool Init(HandleOrRid meshId, HandleOrRid matId);

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