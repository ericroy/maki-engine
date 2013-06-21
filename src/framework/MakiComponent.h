#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	class Entity;
	class Renderer;

	class Component
	{
	public:
		enum Type
		{
			Type_Mesh = 1<<0,
			Type_Light = 1<<2,
			Type_Camera = 1<<3,
			Type_Billboard = 1<<4,
			Type_Character = 1<<5
		};

		static Component *Create(const char *type);

		class Comparator
		{
		public:
			inline bool operator()(const Component *a, const Component *b) const { return a->componentType < b->componentType; }
		};
		
	public:
		Component(Type type) : componentType(type) {}
		virtual ~Component() {}
		
		virtual bool Init(Document::Node *node) = 0;
		
		virtual void Attach(Entity *entity)
		{
			owner = entity;
		}

		inline void Update(float dt)
		{
			if(updateFunc != nullptr) {
				updateFunc(dt);
			}
		}
		
		inline void Draw(Renderer *renderer)
		{
			if(drawFunc != nullptr) {
				drawFunc(renderer);
			}
		}

		inline bool HandleMessage(Component *sender, uint32 message, uintptr_t arg1, uintptr_t arg2)
		{
			if(messageHandlerFunc != nullptr) {
				return messageHandlerFunc(sender, message, arg1, arg2);
			}
			return false;
		}

	public:
		uint64 componentType;
		Entity *owner;

	protected:
		std::function<void(float)> updateFunc;
		std::function<void(Renderer *)> drawFunc;
		std::function<bool(Component *, uint32, uintptr_t, uintptr_t)> messageHandlerFunc;
	};

} // namespace Maki