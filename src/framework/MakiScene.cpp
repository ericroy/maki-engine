#include "framework/framework_stdafx.h"
#include "framework/MakiScene.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiEntityPool.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{

		Scene::Scene(bool prototype) : root(nullptr), drawListHead(nullptr) {
			root = EntityPool::Get()->Create(prototype);
			root->AddComponent(ComponentPoolBase::PoolForType(Component::Type_Transform)->Create());
			root->AddComponent(ComponentPoolBase::PoolForType(Component::Type_SceneNode)->Create());
		}

		Scene::~Scene() {
			EntityPool::Get()->Destroy(root);
		}

		bool Scene::Load(Rid rid) {
			SceneLoader sl;
			return sl.Load(rid, this->root);
		}

		void Scene::UpdateRecursive(Entity *e, const Matrix44 &current)
		{
			Components::SceneNode *nodeComp = e->Get<Components::SceneNode>();
			Components::Transform *transComp = e->Get<Components::Transform>();

			Matrix44 world = current * transComp->GetMatrix();
			transComp->SetWorldMatrix(world);

			const int32 size = nodeComp->children.size();
			for(int32 i = 0; i < size; i++) {
				UpdateRecursive(nodeComp->children[i], world);
			}
		}

	} // namespace Framework

} // namespace Maki