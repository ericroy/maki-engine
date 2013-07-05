#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class SceneNode : public Component
			{
			public:
				static const Type TYPE = Type_SceneNode;
				static const TypeFlag DEPENDENCIES = TypeFlag_Transform;

			public:
				SceneNode();
				virtual ~SceneNode();
				bool Init(Document::Node *props);

				void AddChild(Entity *e);
				void RemoveChild(Entity *e);
				Entity *RemoveChild(int32 index);
				void SetWorldMatrix(const Matrix44 &world);

			public:
				Entity *parent;
				std::vector<Entity *> children;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki