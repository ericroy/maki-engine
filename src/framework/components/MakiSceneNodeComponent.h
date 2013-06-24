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
				friend class SceneGraphSystem;

			public:
				static const Component::Type TYPE = Type_SceneNode;

			public:
				SceneNode();
				virtual ~SceneNode();
				virtual bool Init(Document::Node *node);

				inline void AddChild(Entity *e) { children.push_back(e); e->Get<SceneNode>()->parent = owner; }
				void RemoveChild(Entity *e);

			public:
				Entity *parent;
				std::vector<Entity *> children;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki