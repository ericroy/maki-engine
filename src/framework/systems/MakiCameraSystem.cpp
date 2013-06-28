#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiCameraSystem.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			CameraSystem::CameraSystem()
				: System(Component::TypeFlag_Camera|Component::TypeFlag_Transform)
			{
			}

			CameraSystem::~CameraSystem()
			{
			}

			void CameraSystem::Add(Entity *e)
			{
				Node n;
				n.camComp = e->Get<Components::Camera>();
				n.transComp = e->Get<Components::Transform>();
				nodes.push_back(n);
			}

			void CameraSystem::Remove(Entity *e)
			{
				Node n;
				n.camComp = e->Get<Components::Camera>();
				n.transComp = e->Get<Components::Transform>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki