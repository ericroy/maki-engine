#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiSkeletonSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiSkeletonComponent.h"

namespace Maki
{
	SkeletonSystem::SkeletonSystem()
		: System(Component::Type_Mesh|Component::Type_Skeleton)
	{
	}

	SkeletonSystem::~SkeletonSystem()
	{
	}

	void SkeletonSystem::Update()
	{
		const uint32 count = items.size();
		for(uint32 i = 0; i < count; i++) {
			const Item &item = items[i];
			Material *mat = MaterialManager::Get(item.meshComp->material);
			item.skelComp->Update(mat);
		}
	}

	void SkeletonSystem::Add(Entity *e)
	{
		Item item;
		item.meshComp = e->Get<MeshComponent>();
		item.skelComp = e->Get<SkeletonComponent>();
		items.push_back(item);
	}

	void SkeletonSystem::Remove(Entity *e)
	{
		Item item;
		item.meshComp = e->Get<MeshComponent>();
		item.skelComp = e->Get<SkeletonComponent>();
		items.erase(std::find(std::begin(items), std::end(items), item));
	}


} // namespace Maki