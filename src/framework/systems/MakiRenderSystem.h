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
			class Transform;
			class Mesh;
			class Flash;
		}

		namespace Systems
		{

			class RenderSystem : public System3<Components::Transform, Components::Mesh, Components::Flash>
			{
			public:
				RenderSystem(uint32 messageQueueSize);
				virtual ~RenderSystem();
				void Draw(Core::Renderer *renderer, uint32 requiredFlags = 0, uint32 disallowedFlags = 0, const Frustum *cullingFrustum = nullptr, const Matrix44 *view = nullptr);

			private:
				void CalculateFrustumPlanes(const Frustum *cullingFrustum, const Matrix44 *view, Vector4 frustumPlanes[6]);
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki