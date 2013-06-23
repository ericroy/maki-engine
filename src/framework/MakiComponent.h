#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	class Entity;
	class Renderer;

	class Component
	{
		friend class Entity;

	public:
		enum Type
		{
			Type_Transform = 0,
			Type_SceneNode,
			Type_Mesh,
			Type_Light,
			Type_Camera,
			Type_Billboard,
			Type_Character,
			Type_Skeleton,
			TypeCount
		};

		typedef uint64 Message;
		static const uint64 Message_DebugWidgetCreated = 1LL << 0;

		class Comparator
		{
		public:
			inline bool operator()(const Component *a, const Component *b) const { return a->componentType < b->componentType; }
		};
		
	public:
		Component(Type componentType) : componentType(componentType), owner(nullptr) {}
		virtual ~Component() {}

		virtual bool Init(Document::Node *node) = 0;
		virtual void Attach(Entity *owner) { this->owner = owner; }
		virtual void Detach() { this->owner = nullptr; }

	public:
		Type componentType;
		Entity *owner;
	};

} // namespace Maki