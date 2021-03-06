#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
		class Entity;

		class Component
		{
		public:
			enum Type
			{
				Type_Transform = 0,
				Type_SceneNode,
				Type_Mesh,
				Type_Sprite,
				Type_Flash,
				Type_Meta,
				Type_Physics,
				Type_Script,
				Type_Name,
				Type_NavMesh,
				Type_Light,
				Type_Camera,
				Type_Skeleton,
				Type_UserType,
			};

			typedef uint64 TypeFlag;
			static const TypeFlag TypeFlag_Transform = 1ULL << Type_Transform;
			static const TypeFlag TypeFlag_SceneNode = 1ULL << Type_SceneNode;
			static const TypeFlag TypeFlag_Mesh = 1ULL << Type_Mesh;
			static const TypeFlag TypeFlag_Sprite = 1ULL << Type_Sprite;
			static const TypeFlag TypeFlag_Flash = 1ULL << Type_Flash;
			static const TypeFlag TypeFlag_Meta = 1ULL << Type_Meta;
			static const TypeFlag TypeFlag_Physics = 1ULL << Type_Physics;
			static const TypeFlag TypeFlag_Script = 1ULL << Type_Script;
			static const TypeFlag TypeFlag_Name = 1ULL << Type_Name;
			static const TypeFlag TypeFlag_NavMesh = 1ULL << Type_NavMesh;
			static const TypeFlag TypeFlag_Light = 1ULL << Type_Light;
			static const TypeFlag TypeFlag_Camera= 1ULL << Type_Camera;
			static const TypeFlag TypeFlag_Skeleton = 1ULL << Type_Skeleton;
			static const TypeFlag TypeFlag_UserType = 1ULL << Type_UserType;

			class Comparator
			{
			public:
				inline bool operator()(const Component *a, const Component *b) const { return a->type < b->type; }
			};
		
		public:
			Component(Type type, uint64 dependencies) : type(type), dependencies(dependencies), owner(nullptr) {}
			virtual ~Component() {}
			virtual bool Init(Core::Document::Node *props) { return true; }
			virtual void OnAttach() {}
			virtual void OnDetach() {}
			virtual Component *Clone(bool prototype) = 0;

		public:
			Type type;
			uint64 dependencies;
			Entity *owner;
		};

	} // namespace Framework

} // namespace Maki