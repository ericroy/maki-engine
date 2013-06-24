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

		class RenderSystem : public System
		{
			struct Item
			{
				Components::Mesh *meshComp;
				Components::Transform *transComp;

				inline bool operator==(const Item &other) const { return meshComp == other.meshComp && transComp == other.transComp; }
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


	} // namespace Framework

} // namespace Maki