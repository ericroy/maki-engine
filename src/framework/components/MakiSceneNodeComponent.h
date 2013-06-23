#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class SceneNodeComponent : public Component
	{
		friend class SceneGraphSystem;

	public:
		static const Component::Type COMPONENT_TYPE = Type_SceneNode;

	public:
		SceneNodeComponent();
		virtual ~SceneNodeComponent();
		virtual bool Init(Document::Node *node);

		inline void AddChild(Entity *e) { children.push_back(e); e->Get<SceneNodeComponent>()->parent = owner; }
		void RemoveChild(Entity *e);

	public:
		Entity *parent;
		std::vector<Entity *> children;
	};


}