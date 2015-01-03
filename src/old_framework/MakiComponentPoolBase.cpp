#include "framework/framework_stdafx.h"
#include "framework/MakiComponentPoolBase.h"

namespace Maki
{
	namespace Framework
	{
		uint64 ComponentPoolBase::poolTypes = 0;
		std::vector<ComponentPoolBase *> ComponentPoolBase::poolForType;
		std::map<std::string, ComponentPoolBase *> ComponentPoolBase::poolForTypeName;

		void ComponentPoolBase::Register(const char *typeName, Component::Type type, ComponentPoolBase *pool)
		{
			assert((poolTypes & (1ULL << type)) == 0 && "Pool of this type already registered");
			poolTypes |= (1ULL << type);
			
			int32 enlargeBy = type - poolForType.size() + 1;
			while(enlargeBy > 0) {
				poolForType.push_back(nullptr);
				enlargeBy--;
			}

			poolForType[type] = pool;
			poolForTypeName[typeName] = pool;
		}

		void ComponentPoolBase::Unregister(Component::Type type)
		{
			assert((poolTypes & (1ULL << type)) != 0 && "Pool of this type has not yet been registered");
			poolTypes &= ~(1ULL << type);
			poolForType[type] = nullptr;
		}

		ComponentPoolBase *ComponentPoolBase::PoolForType(Component::Type type)
		{
			if((poolTypes & (1ULL << type)) == 0) {
				return nullptr;
			}
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