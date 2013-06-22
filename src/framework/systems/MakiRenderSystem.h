#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class MeshComponent;
	class SceneNodeComponent;

	class RenderSystem : public System
	{
		struct Item
		{
			MeshComponent *meshComp;
			SceneNodeComponent *nodeComp;
		};

	public:
		RenderSystem();
		virtual ~RenderSystem();
		
		void Draw(Renderer *renderer, uint32 requiredFlags = 0, uint32 disallowedFlags = 0, const Frustum *cullingFrustum = nullptr, const Matrix44 *view = nullptr);

	protected:
		void Add(Entity *e);
		virtual void Remove(Entity *e);

	private:
		void CalculateFrustumPlanes(const Frustum *cullingFrustum, const Matrix44 *view, Vector4 frustumPlanes[6]);

	private:
		std::vector<Item> items;
	};


} // namespace Maki