#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{

		// The purpose of this base class is to allow the freeing of a Component* without knowing
		// the type or which pool it came from
		class ComponentPoolBase
		{
		public:
			static ComponentPoolBase *PoolForType(Component::Type type);
			static ComponentPoolBase *PoolForTypeName(const char *typeName);
			static bool TypeForTypeName(const char *typeName, Component::Type &type);

		protected:
			static void Register(const char *typeName, Component::Type type, ComponentPoolBase *pool);
			static void Unregister(Component::Type type);

		protected:
			static uint64 poolTypes;
			static std::vector<ComponentPoolBase *> poolForType;
			static std::map<std::string, ComponentPoolBase *> poolForTypeName;

		public:
			virtual Component *Create() = 0;
			virtual void Destroy(Component *c) = 0;
			virtual Component::Type GetType() = 0;
		};

	} // namespace Framework

} // namespace Maki