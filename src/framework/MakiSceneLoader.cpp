#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiComponentPoolBase.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiEntityPool.h"
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
				Console::Error("Failed to load doc");
				return false;
			}

			return Recurse(doc.root->Resolve("root"), parentNodeForScene);
		}

		bool SceneLoader::Recurse(Document::Node *n, Entity *container) {
			Document::Node *children = n->Resolve("children");
			if(children == nullptr) {
				return true;
			}

			Components::SceneNode *containerSceneNode = container->Get<Components::SceneNode>();

			for(uint32 i = 0; i < children->count; i++) {
				Document::Node *c = children->children[i];

				if(c->ValueEquals("entity")) {

					Entity *e = EntityPool::Get()->Create();
				
					// Load and init all components for this entity
					for(uint32 ci = 0; ci < c->count; ci++) {
						Document::Node *compNode = c->children[ci];
						if(compNode->ValueEquals("children")) {
							continue;
						}

						ComponentPoolBase *pool = ComponentPoolBase::PoolForTypeName(compNode->value);
						if(pool == nullptr) {
							Console::Error("There is no pool for components of type: %s", compNode->value);
							return false;
						}
						Component *comp = pool->Create();
						if(!comp->Init(compNode)) {
							Console::Error("Failed to intialize component of type: %s", compNode->value);
							pool->Destroy(comp);
							return false;
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
					if(!Recurse(c, e)) {
						return false;
					}

				} else if(c->ValueEquals("scene")) {

					const char *scenePath = c->ResolveValue("#0");
					if(scenePath == nullptr) {
						Console::Error("Embedded scene reference must specify a path");
						return false;
					}

					Rid sceneRid = Engine::Get()->assets->PathToRid(scenePath);
					if(sceneRid == RID_NONE) {
						Console::Error("Embedded scene path could not be resolved to RID");
						return false;
					}

					SceneLoader loader;
					if(!loader.Load(sceneRid, container)) {
						Console::Error("Failed to load embedded scene Rid<%d>", sceneRid);
						return false;
					}

				} else {

					Console::Error("Unrecognized scene node: %s", c->value);
					return false;

				}
			}

			return true;
		}
	
	} // namespace Framework

} // namespace Maki