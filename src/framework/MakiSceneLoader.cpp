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

	bool SceneLoader::Load(Rid rid, Entity *parentNodeForScene, std::vector<Camera *> *cameras, std::vector<Light *> *lights, std::vector<Entity *> *physicsHulls) {
		Engine *eng = Engine::Get();

		Document doc;
		if(!doc.Load(rid)) {
			return false;
		}

		Recurse(doc.root->Resolve("root"), parentNodeForScene, cameras, lights, physicsHulls);
		return true;
	}

	void SceneLoader::Recurse(Document::Node *n, Entity *container, std::vector<Camera *> *cameras, std::vector<Light *> *lights, std::vector<Entity *> *physicsHulls) {
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
					e = Create<Entity, EntityFactory>(c);
				} else if(c->ValueEquals("mesh_entity")) {
					e = Create<MeshEntity, MeshEntityFactory>(c);
				} else if(c->ValueEquals("skinned_mesh_entity")) {
					e = Create<SkinnedMeshEntity, SkinnedMeshEntityFactory>(c);
				} else if(c->ValueEquals("character_entity")) {
					e = Create<CharacterEntity, CharacterEntityFactory>(c);
				} else if(c->ValueEquals("billboard_entity")) {
					e = Create<BillboardEntity, BillboardEntityFactory>(c);
				} else if(c->ValueEquals("light")) {
					e = Create<Light, LightFactory>(c);
					if(e != nullptr && lights != nullptr) {
						lights->push_back((Light *)e);
					}
				} else if(c->ValueEquals("camera")) {
					e = Create<Camera, CameraFactory>(c);
					if(e != nullptr && cameras != nullptr) {
						cameras->push_back((Camera *)e);
					}
				} else {
					Console::Warning("Don't know how to parse node type '%s'", c->value);
				}

				if(physicsHulls != nullptr && e->GetFlag(Entity::Flag_Physics)) {
					physicsHulls->push_back(e);
				}

				if(e != nullptr) {
					container->AddChild(e);
					Recurse(c, e, cameras, lights, physicsHulls);
				}
			}
		}
	}
	
} // namespace Maki