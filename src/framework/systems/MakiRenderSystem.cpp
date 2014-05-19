#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiFlashComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			RenderSystem::RenderSystem(uint32 messageQueueSize)
				: System(Component::TypeFlag_Transform, Component::TypeFlag_Mesh|Component::TypeFlag_Flash, messageQueueSize, "RenderSystem")
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
					Components::Transform *transComp = std::get<0>(nodes[i]);
					Components::Mesh *meshComp = std::get<1>(nodes[i]);
					Components::Flash *flashComp = std::get<2>(nodes[i]);

					RenderableComponent *renderComp;
					if(meshComp != nullptr) {
						renderComp = static_cast<RenderableComponent *>(meshComp);
					} else {
						renderComp = static_cast<RenderableComponent *>(flashComp);
					}
					
					// Only consider rendering meshes that are compatible with the provided masks
					if((renderComp->renderFlags & requiredFlags) != requiredFlags || (renderComp->renderFlags & disallowedFlags) != 0) {
						continue;
					}

					const Matrix44 &world = transComp->GetWorldMatrix();

					// Perform culling if necessary
					if(cullingFrustum != nullptr && !renderComp->bounds.empty) {
						BoundingSphere worldBounds(world * (renderComp->bounds.pos * renderComp->meshScale), renderComp->bounds.GetRadius() * renderComp->meshScale);

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
				
									
					Matrix44 m = world * renderComp->scaleMatrix;

					if(meshComp != nullptr) {
						// Submit all the draw commands for this mesh
						const uint32 count = meshComp->drawCommands.count;
						for(uint32 i = 0; i < count; i++) {
#if _DEBUG
							// I don't want to set this every draw call for efficiency reasons, but if we don't
							// then hot swapping materials doesn't have any effect.  Perhaps we'll just leave this on
							// in debug mode for now
							meshComp->drawCommands[i].SetMaterial(meshComp->material);
#endif
							renderer->Draw(meshComp->drawCommands[i], m);
						}

					} else {

						const FlashMovieState &state = flashComp->state;
						for(uint32 i = 0; i < state.groups.count; i++) {
							renderer->Draw(state.groups[i].dc, m);
						}
#if _DEBUG
						if(state.metaGroup.mesh != HANDLE_NONE) {
							renderer->Draw(state.metaGroup.dc, m);
						}
#endif
					}
					
				}
				
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