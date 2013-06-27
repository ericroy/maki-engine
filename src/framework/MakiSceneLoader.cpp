#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiComponentPoolBase.h"
#include "framework/MakiComponentPool.h"
#include "framework/components/MakiSceneNodeComponent.h"


namespace Maki
{
	namespace Framework
	{

		SceneLoader::SceneLoader() {
		}

		SceneLoader::~SceneLoader() {
		}

		bool SceneLoader::Load(Rid rid, Entity *parentNodeForScene) {
			Engine *eng = Engine::Get();

			Document doc;
			if(!doc.Load(rid)) {
				return false;
			}

			Recurse(doc.root->Resolve("root"), parentNodeForScene);
			return true;
		}

		void SceneLoader::Recurse(Document::Node *n, Entity *container) {
			if(n == nullptr) {
				return;
			}

			Document::Node *children = n->Resolve("children");
			if(children == nullptr) {
				return;
			}

			Components::SceneNode *containerSceneNode = container->Get<Components::SceneNode>();

			for(uint32 i = 0; i < children->count; i++) {
				Document::Node *c = children->children[i];
				if(!c->ValueEquals("entity")) {
					Console::Error("Expected an entity, but got node with value: %s", c->value);
					continue;
				}
					
				Entity *e = new Entity();
						
				// Load and init all components for this entity
				for(uint32 ci = 0; ci < c->count; ci++) {
					Document::Node *compNode = c->children[ci];
					ComponentPoolBase *pool = ComponentPoolBase::PoolForTypeName(compNode->value);
					if(pool == nullptr) {
						Console::Error("There is no pool for components of type: %s", compNode->value);
						continue;
					}
					Component *comp = pool->Create();
					if(!comp->Init(compNode)) {
						Console::Error("Failed to intialize component of type: %s", compNode->value);
						pool->Destroy(comp);
						continue;
					}
					e->AddComponent(comp);
				}
						
				// Ensure that the entity has the components necessary to be a part of a scene graph
				if(!e->HasComponent(Component::Type_Transform)) {
					e->AddComponent(ComponentPoolBase::PoolForType(Component::Type_Transform)->Create());	
				}
				if(!e->HasComponent(Component::Type_SceneNode)) {
					e->AddComponent(ComponentPoolBase::PoolForType(Component::Type_SceneNode)->Create());
				}

				// Hook the entity up to the scene graph, and then recursively process its children
				containerSceneNode->AddChild(e);
				Recurse(c, e);
			}
		}
	
	} // namespace Framework

} // namespace Maki