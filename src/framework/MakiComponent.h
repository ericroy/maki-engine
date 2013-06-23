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
		static const uint64 Type_Transform = 1L << 0;
		static const uint64 Type_SceneNode = 1L << 1;
		static const uint64 Type_Mesh = 1L << 2;
		static const uint64 Type_Light = 1L << 3;
		static const uint64 Type_Camera = 1L << 4;
		static const uint64 Type_Billboard = 1L << 5;
		static const uint64 Type_Character = 1L << 6;
		static const uint64 Type_Skeleton = 1L << 7;

		static const uint64 Message_DebugWidgetCreated = 1L << 0;
		static const uint64 Message_MeshBoundsChanged = 1L << 1;
		static const uint64 MessageAll = 0xffffffffffffffff;

		static Component *Create(const char *type);

		class Comparator
		{
		public:
			inline bool operator()(const Component *a, const Component *b) const { return a->componentType < b->componentType; }
		};
		
	public:
		Component(uint64 type) : componentType(type), owner(nullptr), messageHandler(nullptr) {}
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
		std::function<bool(Component *, uint64, uintptr_t, uintptr_t)> messageHandler;
	};

} // namespace Maki