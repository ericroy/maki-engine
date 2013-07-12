#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Mesh;
			class Skeleton;
		}

		namespace Systems
		{

			class SkeletonSystem : public System
			{
				struct  Node
				{
					Components::Skeleton *skelComp;
					Components::Mesh *meshComp;

					inline bool operator==(const Node &other) const { return skelComp == other.skelComp; }
				};

			public:
				SkeletonSystem();
				virtual ~SkeletonSystem();
		
				void Update();

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki