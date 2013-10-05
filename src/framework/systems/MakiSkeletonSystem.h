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

			class SkeletonSystem : public System2<Components::Skeleton, Components::Mesh>
			{
			public:
				SkeletonSystem(uint32 messageQueueSize);
				virtual ~SkeletonSystem();
				void Update();
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki