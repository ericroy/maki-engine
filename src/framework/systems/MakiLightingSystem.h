#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiLightComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;
			class Light;
		}

		namespace Systems
		{

			class LightingSystem : public System
			{
			private:
				struct  Node
				{
					Components::Light *lightComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return lightComp == other.lightComp; }
				};

			public:
				LightingSystem();
				virtual ~LightingSystem();
		
				inline uint32 GetLightCount() const { return nodes.size(); }
				inline Entity *GetLight(uint32 index) { return nodes[index].lightComp->owner; }

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki