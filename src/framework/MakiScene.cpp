#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"
#include "framework/MakiCamera.h"
#include "framework/MakiLight.h"
#include "framework/MakiSceneLoader.h"

namespace Maki
{

	Scene::Scene() : root(nullptr), drawListHead(nullptr) {
		root = new Entity();
	}

	Scene::~Scene() {
		SAFE_DELETE(root);
	}

	bool Scene::Load(Rid rid) {
		SceneLoader sl;
		return sl.Load(rid, this->root, &this->cameras, &this->lights, &this->physicsHulls);
	}

	void Scene::Update(float dt) {
		drawListHead = nullptr;
		root->RecursivelyUpdate(&drawListHead, Matrix44::Identity, dt);
	}

	void Scene::Draw(Renderer *renderer, uint32 requiredFlags, uint32 disallowedFlags, const Frustum *cullingFrustum, const Matrix44 *view) {
		Vector4 frustumPlanes[6];
		if(cullingFrustum != nullptr) {
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
		
		for(Entity *e = drawListHead; e != nullptr; e = e->drawListNext) {
			uint32 f = e->GetFlags();
			if((f & requiredFlags) == requiredFlags && (f & disallowedFlags) == 0) {
				if(cullingFrustum != nullptr && !e->bounds.empty) {
					BoundingSphere worldBounds(e->GetWorldMatrix() * e->bounds.pos, e->bounds.GetRadius());

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
				e->Draw(renderer);
			}
		}
	}




} // namespace Maki