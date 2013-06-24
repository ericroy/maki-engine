#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
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
			static const uint64 Message_DebugWidgetCreated = 1ULL << 0;

			class Comparator
			{
			public:
				inline bool operator()(const Component *a, const Component *b) const { return a->type < b->type; }
			};
		
		public:
			Component(Type type) : type(type), owner(nullptr) {}
			virtual ~Component() {}

		public:
			Type type;
			Entity *owner;
		};

	} // namespace Framework

} // namespace Maki