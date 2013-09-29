#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiFlashComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;
			class Flash;
		}

		namespace Systems
		{

			class FlashSystem : public System
			{
			public:
				FlashSystem(uint32 messageQueueSize);
				virtual ~FlashSystem();
		
				void Update(float dt);

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Components::Flash *> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki