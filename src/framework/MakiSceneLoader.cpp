#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
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

					container->AddChild(e);
					Recurse(c, e, cameras, lights, physicsHulls);

				} else {
					Console::Error("Don't know how to process scene node with value: %s", c->value);
				}
			}
		}
	}
	
} // namespace Maki