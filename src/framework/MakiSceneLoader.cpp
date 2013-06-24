#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiCameraComponent.h"
#include "framework/components/MakiSceneNodeComponent.h"




//
//Document::Node *flagsNode = node->Resolve("flags");
//		if(flagsNode != nullptr) {
//			for(uint32 i = 0; i < flagsNode->count; i++) {
//				char *value = flagsNode->children[i]->value;
//				bool on = true;
//				if(value[0] == '!') {
//					on = false;
//					value++;
//				}
//				#define __SET(X) if(on) { flags |= X; } else { flags &= ~X; }
//				if(strcmp(value, "draw") == 0) __SET(Entity::Flag_Draw)
//				else if(strcmp(value, "update") == 0) __SET(Entity::Flag_Update)
//				else if(strcmp(value, "physics") == 0) __SET(Entity::Flag_Physics)
//				else if(strcmp(value, "process_children") == 0) __SET(Entity::Flag_ProcessChildren)
//				else if(strcmp(value, "cast_shadow") == 0) __SET(Entity::Flag_CastShadow)
//				#undef __SET
//			}
//		}
//





namespace Maki
{
	namespace Framework
	{

		SceneLoader::SceneLoader() {
		}

		SceneLoader::~SceneLoader() {
		}

		bool SceneLoader::Load(Rid rid, Entity *parentNodeForScene, std::vector<Entity *> *cameras, std::vector<Entity *> *lights, std::vector<Entity *> *physicsHulls) {
			Engine *eng = Engine::Get();

			Document doc;
			if(!doc.Load(rid)) {
				return false;
			}

			Recurse(doc.root->Resolve("root"), parentNodeForScene, cameras, lights, physicsHulls);
			return true;
		}

		void SceneLoader::Recurse(Document::Node *n, Entity *container, std::vector<Entity *> *cameras, std::vector<Entity *> *lights, std::vector<Entity *> *physicsHulls) {
			if(n == nullptr) {
				return;
			}
			Document::Node *children = n->Resolve("children");
			if(children == nullptr) {
				return;
			}
			for(uint32 i = 0; i < children->count; i++) {
				Document::Node *c = children->children[i];
				if(c != nullptr) {
					if(c->ValueEquals("entity")) {
						Entity *e = new Entity();
						/*
						if(!e->Init(c)) {
							Console::Error("Entity init failed");
							delete e;
							continue;
						}
				
						if(lights != nullptr && e->Get<LightComponent>() != nullptr) {
							lights->push_back(e);
						}
						if(cameras != nullptr && e->Get<CameraComponent>() != nullptr) {
							cameras->push_back(e);
						}
						if(physicsHulls != nullptr && e->GetFlag(Entity::Flag_Physics)) {
							physicsHulls->push_back(e);
						}

						container->Get<SceneNodeComponent>()->AddChild(e);*/
						Recurse(c, e, cameras, lights, physicsHulls);

					} else {
						Console::Error("Don't know how to process scene node with value: %s", c->value);
					}
				}
			}
		}
	
	} // namespace Framework

} // namespace Maki