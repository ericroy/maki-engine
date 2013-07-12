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

			class FlyCamSystem : public System
			{
			private:
				struct  Node
				{
					Components::Camera *camComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return camComp == other.camComp; }
				};

			public:
				FlyCamSystem();
				virtual ~FlyCamSystem();
		
				void Update(float dt);

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki