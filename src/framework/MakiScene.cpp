#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"
#include "framework/MakiCamera.h"
#include "framework/MakiLight.h"
#include "framework/MakiSceneLoader.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"

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

	void Scene::UpdateRecursive(Entity *e, const Matrix44 &current, float dt)
	{
		SceneNodeComponent *nodeComp = e->Get<SceneNodeComponent>();
		TransformComponent *transComp = e->Get<TransformComponent>();

		transComp->useWorldMatrix = true;
		transComp->world = current * transComp->GetMatrix();
		
		if(e->GetFlag(Entity::Flag_ProcessChildren)) {
			const int32 size = nodeComp->children.size();
			for(int32 i = 0; i < size; i++) {
				UpdateRecursive(nodeComp->children[i], transComp->world, dt);
			}
		}
	}

} // namespace Maki