#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiComponentPool.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{

		Scene::Scene() : root(nullptr), drawListHead(nullptr) {
			root = new Entity();
			root->AddComponent(ComponentPoolBase::PoolForType(Component::Type_Transform)->Create());
			root->AddComponent(ComponentPoolBase::PoolForType(Component::Type_SceneNode)->Create());
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
			Components::SceneNode *nodeComp = e->Get<Components::SceneNode>();
			Components::Transform *transComp = e->Get<Components::Transform>();

			Matrix44 world = current * transComp->GetMatrix();
			transComp->SetWorldMatrix(world);

			const int32 size = nodeComp->children.size();
			for(int32 i = 0; i < size; i++) {
				UpdateRecursive(nodeComp->children[i], world, dt);
			}
		}

	} // namespace Framework

} // namespace Maki