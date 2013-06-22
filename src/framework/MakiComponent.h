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
			Type_SceneNode = 1<<0,
			Type_Mesh = 1<<1,
			Type_Light = 1<<2,
			Type_Camera = 1<<3,
			Type_Billboard = 1<<4,
			Type_Character = 1<<5,
			Type_Skeleton = 1<<6,
		};

		enum Message
		{
			Message_DebugWidgetCreated = 0,
			Message_MeshBoundsChanged,
			MessageCount
		};

		static Component *Create(const char *type);

		class Comparator
		{
		public:
			inline bool operator()(const Component *a, const Component *b) const { return a->componentType < b->componentType; }
		};
		
	public:
		Component(Type type) : componentType(type), owner(nullptr), messageHandler(nullptr) {}
		virtual ~Component() {}
		virtual bool Init(Document::Node *node) = 0;

	protected:
		virtual void Attach(Entity *entity)
		{
			owner = entity;
		}
		
		virtual void Detach()
		{
			owner = nullptr;
		}

	public:
		uint64 componentType;
		Entity *owner;
		std::function<bool(Component *, Message, uintptr_t, uintptr_t)> messageHandler;
	};

} // namespace Maki