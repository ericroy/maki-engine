#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiMeshEntity.h"
#include "framework/MakiSkinnedMeshEntity.h"
#include "framework/MakiCharacterEntity.h"
#include "framework/MakiBillboardEntity.h"
#include "framework/MakiLight.h"
#include "framework/MakiCamera.h"

namespace Maki
{
	template<class EntityType, class FactoryType>
	inline EntityType *Create(Document::Node *node)
	{
		FactoryType factory;
		if(!factory.PreCreate(node)) {
			Console::Error("Factory PreCreate failed");
			return nullptr;
		}
		EntityType *e = factory.Create();
		factory.PostCreate(e);
		return e;
	}


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
				Entity *e = nullptr;
				if(c->ValueEquals("entity")) {
					e = new Entity();
					if(!e->Init(c)) {
						Console::Error("Entity init failed");
					}
				} else if(c->ValueEquals("light")) {
					e = new Entity();
					if(!e->Init(c)) {
						Console::Error("Light init failed");
					} else if(lights != nullptr) {
						lights->push_back(e);
					}
				} else if(c->ValueEquals("camera")) {
					e = new Entity();
					if(!e->Init(c)) {
						Console::Error("Camera init failed");
					} else if(lights != nullptr) {
						cameras->push_back(e);
					}
				} else {
					Console::Warning("Don't know how to parse node type '%s'", c->value);
				}

				if(e != nullptr) {
					if(physicsHulls != nullptr && e->GetFlag(Entity::Flag_Physics)) {
						physicsHulls->push_back(e);
					}

					container->AddChild(e);
					Recurse(c, e, cameras, lights, physicsHulls);
				}
			}
		}
	}
	
} // namespace Maki