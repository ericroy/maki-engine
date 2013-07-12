#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Transform;
			class Camera;
		}

		namespace Systems
		{

			class CameraSystem : public System
			{
			private:
				struct  Node
				{
					Components::Camera *camComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return camComp == other.camComp; }
				};

			public:
				CameraSystem();
				virtual ~CameraSystem();
		
				inline uint32 GetCameraCount() const { return nodes.size(); }
				inline Entity *GetCamera(uint32 index) { return nodes[index].camComp->owner; }

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki