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
			class SceneNode;
		}

		namespace Systems
		{
			class SceneGraphSystem : public System
			{
			public:
				SceneGraphSystem();
				virtual ~SceneGraphSystem();

			protected:
				void Add(Entity *e);
				virtual void Remove(Entity *e);

			private:
				std::vector<Components::SceneNode *> nodes;
			};
		
		} // namespace Systems

	} // namespace Framework

} // namespace Maki