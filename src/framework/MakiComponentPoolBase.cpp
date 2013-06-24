#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponentPoolBase.h"

namespace Maki
{
	namespace Framework
	{

		ComponentPoolBase *ComponentPoolBase::poolForType[Component::TypeCount];
		std::map<std::string, ComponentPoolBase *> ComponentPoolBase::poolForTypeName;

		ComponentPoolBase *ComponentPoolBase::PoolForType(Component::Type type)
		{
			return poolForType[type];
		}

		ComponentPoolBase *ComponentPoolBase::PoolForTypeName(const char *typeName)
		{
			auto iter = poolForTypeName.find(typeName);
			if(iter == poolForTypeName.end())
			{
				return nullptr;
			}
			return iter->second;
		}

	} // namespace Framework

} // namespace Maki