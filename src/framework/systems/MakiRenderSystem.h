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
			class Transform;
		}

		namespace Systems
		{

			class RenderSystem : public System
			{
				struct  Node
				{
					Components::Mesh *meshComp;
					Components::Transform *transComp;

					inline bool operator==(const Node &other) const { return meshComp == other.meshComp; }
				};

			public:
				RenderSystem(uint32 messageQueueSize);
				virtual ~RenderSystem();
		
				void Draw(Core::Renderer *renderer, uint32 requiredFlags = 0, uint32 disallowedFlags = 0, const Frustum *cullingFrustum = nullptr, const Matrix44 *view = nullptr);

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				void CalculateFrustumPlanes(const Frustum *cullingFrustum, const Matrix44 *view, Vector4 frustumPlanes[6]);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki