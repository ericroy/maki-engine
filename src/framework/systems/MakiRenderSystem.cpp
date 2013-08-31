#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			RenderSystem::RenderSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Mesh|Component::TypeFlag_Transform, messageQueueSize, "RenderSystem")
			{
			}

			RenderSystem::~RenderSystem()
			{
			}

			void RenderSystem::Draw(Core::Renderer *renderer, uint32 requiredFlags, uint32 disallowedFlags, const Frustum *cullingFrustum, const Matrix44 *view)
			{
				
				Vector4 frustumPlanes[6];
				if(cullingFrustum != nullptr) {
					CalculateFrustumPlanes(cullingFrustum, view, frustumPlanes);
				}

				const uint32 count = nodes.size();
				for(uint32 i = 0; i < count; i++) {
					const Node &n = nodes[i];

					// Only consider rendering meshes that are compatible with the provided masks
					if((n.meshComp->flags & requiredFlags) == requiredFlags && (n.meshComp->flags & disallowedFlags) == 0) {
				
						const Matrix44 &world = n.transComp->GetWorldMatrix();

						// Perform culling if necessary
						if(cullingFrustum != nullptr && !n.meshComp->bounds.empty) {
							BoundingSphere worldBounds(world * (n.meshComp->bounds.pos * n.meshComp->meshScale), n.meshComp->bounds.GetRadius() * n.meshComp->meshScale);

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
						Matrix44 m = world * n.meshComp->scaleMatrix;
						const uint32 count = n.meshComp->drawCommands.count;
						for(uint32 i = 0; i < count; i++) {
#if _DEBUG
							// I don't want to set this every draw call for efficiency reasons, but if we don't
							// then hot swapping materials doesn't have any effect.  Perhaps we'll just leave this on
							// in debug mode for now
							n.meshComp->drawCommands[i].SetMaterial(n.meshComp->material);
#endif
							renderer->Draw(n.meshComp->drawCommands[i], m);
						}

					}
				}
				
			}

			void RenderSystem::Add(Entity *e)
			{
				Node n;
				n.meshComp = e->Get<Components::Mesh>();
				n.transComp = e->Get<Components::Transform>();
				nodes.push_back(n);
			}

			void RenderSystem::Remove(Entity *e)
			{
				Node n;
				n.meshComp = e->Get<Components::Mesh>();
				n.transComp = e->Get<Components::Transform>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
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

		} // namespace Systems

	} // namespace Framework

} // namespace Maki