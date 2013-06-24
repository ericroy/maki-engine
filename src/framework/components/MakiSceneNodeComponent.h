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
				static const Component::Type TYPE = Type_SceneNode;

			public:
				SceneNode();
				virtual ~SceneNode();
				bool Init(Document::Node *props);

				void AddChild(Entity *e);
				void RemoveChild(Entity *e);
				void SetWorldMatrix(const Matrix44 &world);

			public:
				Entity *parent;
				std::vector<Entity *> children;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki