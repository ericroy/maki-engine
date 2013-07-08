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

			std::map<std::string, Entity *> prototypes;

			bool ret = true;
			for(uint32 i = 0; i < doc.root->count; i++) {
				ret = Recurse(doc.root->children[i], parentNodeForScene, prototypes);
				if(!ret) {
					break;
				}
			}

			// Cleanup any prototypes that were loaded
			EntityPool *pool = EntityPool::Get();
			for(auto iter = prototypes.begin(); iter != prototypes.end(); ++iter) {
				pool->Destroy(iter->second);
			}

			return ret;
		}

		bool SceneLoader::Recurse(Document::Node *n, Entity *container, std::map<std::string, Entity *> prototypes) {

			Components::SceneNode *containerSceneNode = container->Get<Components::SceneNode>();

			if(n->ValueEquals("entity")) {

				Entity *e = nullptr;
				Document::Node *childrenNode = nullptr;

				Entity *proto = nullptr;
				int32 protoCompIndex = 0;

				// Load and init all components for this entity
				for(uint32 i = 0; i < n->count; i++) {
					Document::Node *compNode = n->children[i];

					// If this entity is derived from a prototype, either get or load the prototype, and initialize the 'proto' pointer above
					if(compNode->ValueEquals("prototype")) {
							
						if(i != 0) {
							Console::Error("Prototype node must be the first child of the entity");
							return false;
						}
							
						const char *protoPath = compNode->ResolveValue("#0");
						if(protoPath == nullptr) {
							Console::Error("Entity must specify prototype path");
							return false;
						}

						auto iter = prototypes.find(protoPath);
						if(iter != prototypes.end()) {

							proto = iter->second;

						} else {

							// We don't have this prototype loaded yet, load it now.
							Rid protoRid = Engine::Get()->assets->PathToRid(protoPath);
							if(protoRid == RID_NONE) {
								Console::Error("Prototype path could not be resolved to RID");
								return false;
							}

							Scene temp;
							SceneLoader loader;
							if(!loader.Load(protoRid, temp.root)) {
								Console::Error("Failed to load prototype Rid<%d>", protoRid);
								return false;
							}

							Components::SceneNode *sceneNodeComp = temp.root->Get<Components::SceneNode>();
							if(sceneNodeComp->children.size() != 1) {
								Console::Error("Prototype scene must contain exactly one entity at the root of the document Rid<%u>", protoRid);
								return false;
							}
							proto = sceneNodeComp->RemoveChild(0);

						}
						continue;
					}

					if(e == nullptr) {
						e = EntityPool::Get()->Create(container->IsPrototype());
					}

					// Skip child node for now, until we have created all components for our current entity.
					if(compNode->ValueEquals("children")) {
						childrenNode = compNode;
						continue;
					}
						
					// If we get this far, then we expect the node to specify a component.
					ComponentPoolBase *pool = ComponentPoolBase::PoolForTypeName(compNode->value);
					if(pool == nullptr) {
						Console::Error("There is no pool for components of type: %s", compNode->value);
						return false;
					}

					if(proto != nullptr) {
						while(protoCompIndex < proto->componentCount && proto->components[protoCompIndex].c->type < pool->GetType()) {
							e->AddComponent(proto->components[protoCompIndex++].c->Clone(container->IsPrototype()));
						}
						if(protoCompIndex < proto->componentCount && proto->components[protoCompIndex].c->type == pool->GetType()) {
							protoCompIndex++;
						}
					}

					Component *comp = pool->Create();
					if(!comp->Init(compNode)) {
						Console::Error("Failed to intialize component of type: %s", compNode->value);
						pool->Destroy(comp);
						return false;
					}
					if(e->HasComponent(comp->type)) {
						Console::Info("test");
					}
					e->AddComponent(comp);					
				}

				if(proto != nullptr) {
					while(protoCompIndex < proto->componentCount) {
						e->AddComponent(proto->components[protoCompIndex++].c->Clone(container->IsPrototype()));
					}
				}

				// Ensure that the entity has at least the components necessary to be a part of a scene graph
				if(!e->HasComponent(Component::Type_Transform)) {
					e->AddComponent(ComponentPoolBase::PoolForType(Component::Type_Transform)->Create());	
				}
				if(!e->HasComponent(Component::Type_SceneNode)) {
					e->AddComponent(ComponentPoolBase::PoolForType(Component::Type_SceneNode)->Create());
				}

				// Hook the entity up to the scene graph, and then recursively process its children
				containerSceneNode->AddChild(e);
				if(childrenNode != nullptr) {
					for(uint32 ci = 0; ci < childrenNode->count; ci++) {
						if(!Recurse(childrenNode->children[ci], e, prototypes)) {
							// Any failure loading a child node should cause the entire scene load to fail immediately.
							return false;
						}
					}
				}

			} else if(n->ValueEquals("scene")) {

				const char *scenePath = n->ResolveValue("#0");
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
				Console::Error("Unrecognized scene node: %s", n->value);
				return false;
			}

			return true;
		}
	
	} // namespace Framework

} // namespace Maki