#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class MeshComponent;
	class SkeletonComponent;

	class SkeletonSystem : public System
	{
		struct Item
		{
			MeshComponent *meshComp;
			SkeletonComponent *skelComp;

			inline bool operator==(const Item &other) const { return meshComp == other.meshComp && skelComp == other.skelComp; }
		};

	public:
		SkeletonSystem();
		virtual ~SkeletonSystem();
		
		void Update();

	protected:
		void Add(Entity *e);
		virtual void Remove(Entity *e);

	private:
		std::vector<Item> items;
	};


} // namespace Maki