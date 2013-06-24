#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{

		RenderSystem::RenderSystem()
			: System(Component::Type_Mesh)
		{
		}

		RenderSystem::~RenderSystem()
		{
		}

		void RenderSystem::Draw(Renderer *renderer, uint32 requiredFlags, uint32 disallowedFlags, const Frustum *cullingFrustum, const Matrix44 *view)
		{
			/*
			Vector4 frustumPlanes[6];
			if(cullingFrustum != nullptr) {
				CalculateFrustumPlanes(cullingFrustum, view, frustumPlanes);
			}

			const uint32 count = items.size();
			for(uint32 i = 0; i < count; i++) {
				const Item &item = items[i];

				// Only consider rendering meshes that are compatible with the provided masks
				uint32 f = item.meshComp->owner->GetFlags();
				if((f & requiredFlags) == requiredFlags && (f & disallowedFlags) == 0) {
				
					const Matrix44 &world = item.transComp->useWorldMatrix ? item.transComp->world : item.transComp->GetMatrix();

					// Perform culling if necessary
					if(cullingFrustum != nullptr && !item.meshComp->bounds.empty) {
						BoundingSphere worldBounds(world * (item.meshComp->bounds.pos * item.meshComp->meshScale), item.meshComp->bounds.GetRadius() * item.meshComp->meshScale);

						bool cull = false;
						for(uint32 i = 0; i < 6; i++) {
							float d = worldBounds.pos.x * frustumPlanes[i].x + worldBounds.pos.y * frustumPlanes[i].y + worldBounds.pos.z * frustumPlanes[i].z;
							if(d - frustumPlanes[i].w > worldBounds.radius) {
								cull = true;
								break;
							}
						}
						if(cull) {
							continue;
						}
					}
				
				
					// Submit all the draw commands for this mesh
					Matrix44 m = world * item.meshComp->scaleMatrix;
					const uint32 count = item.meshComp->drawCommands.count;
					for(uint32 i = 0; i < count; i++) {
	#if _DEBUG
						// I don't want to set this every draw call for efficiency reasons, but if we don't
						// then hot swapping materials doesn't have any effect.  Perhaps we'll just leave this on
						// in debug mode for now
						item.meshComp->drawCommands[i].SetMaterial(item.meshComp->material);
	#endif
						renderer->Draw(item.meshComp->drawCommands[i], m);
					}

				}
			}
			*/
		}

		void RenderSystem::Add(Entity *e)
		{
			Item item;
			item.meshComp = e->Get<Components::Mesh>();
			item.transComp = e->Get<Components::Transform>();
			items.push_back(item);
		}

		void RenderSystem::Remove(Entity *e)
		{
			Item item;
			item.meshComp = e->Get<Components::Mesh>();
			item.transComp = e->Get<Components::Transform>();
			items.erase(std::find(std::begin(items), std::end(items), item));
		}

		void RenderSystem::CalculateFrustumPlanes(const Frustum *cullingFrustum, const Matrix44 *view, Vector4 frustumPlanes[6])
		{
			assert(view != nullptr);
			cullingFrustum->ExtractPlanes(frustumPlanes);
			
			// Transform planes by inverse view matrix to get the planes in world space (where we will do the culling)
			Matrix44 invView;
			Matrix44::AffineInverse(*view, invView);
			for(uint32 i = 0; i < 6; i++) {
				// Calculate a point on the plane in world space
				Vector4 p = frustumPlanes[i] * frustumPlanes[i].w;
				p.w = 1.0f;
				p = invView * p;

				// Transform the plane normal to world space
				frustumPlanes[i].w = 0.0f;
				frustumPlanes[i] = invView * frustumPlanes[i];

				// Set the distance for the plane in world space
				frustumPlanes[i].w = p.Dot(frustumPlanes[i]);
			}
		}


	} // namespace Framework

} // namespace Maki