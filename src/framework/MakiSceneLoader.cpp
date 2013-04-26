#include "framework/framework_stdafx.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiMeshEntity.h"
#include "framework/MakiSkinnedMeshEntity.h"
#include "framework/MakiCharacterEntity.h"
#include "framework/MakiLight.h"
#include "framework/MakiCamera.h"

namespace Maki
{

	SceneLoader::SceneLoader() {
	}

	SceneLoader::~SceneLoader() {
	}

	bool SceneLoader::Load(Rid rid, Entity *parentNodeForScene, std::vector<Camera *> *cameras, std::vector<Light *> *lights, std::vector<Entity *> *physicsHulls) {
		auto eng = Engine::Get();

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
					e = CreateEntity(c);
				} else if(c->ValueEquals("mesh_entity")) {
					e = CreateMeshEntity(c);
				} else if(c->ValueEquals("skinned_mesh_entity")) {
					e = CreateSkinnedMeshEntity(c);
				} else if(c->ValueEquals("character_entity")) {
					e = CreateCharacterEntity(c);
				} else if(c->ValueEquals("light")) {
					e = CreateLight(c);
					if(e != nullptr && lights != nullptr) {
						lights->push_back((Light *)e);
					}
				} else if(c->ValueEquals("camera")) {
					e = CreateCamera(c);
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

	void SceneLoader::UpdateFlags(Document::Node *flagsNode, uint32 &flags) {
		for(uint32 i = 0; i < flagsNode->count; i++) {
			char *p = flagsNode->children[i]->value;
			bool on = true;
			if(*p == '!') { on = false; p++; }
			std::string token = p;

#define UPDATE_FLAGS_FOR_TOKEN(_TOK, _FLAG_VAL) if(token == _TOK) { if(on) { flags |= _FLAG_VAL; } else { flags &= ~_FLAG_VAL; } }
			UPDATE_FLAGS_FOR_TOKEN("draw", Entity::Flag_Draw);
			UPDATE_FLAGS_FOR_TOKEN("update", Entity::Flag_Update);
			UPDATE_FLAGS_FOR_TOKEN("physics", Entity::Flag_Physics);
			UPDATE_FLAGS_FOR_TOKEN("process_children", Entity::Flag_ProcessChildren);
			UPDATE_FLAGS_FOR_TOKEN("cast_shadow", Entity::Flag_CastShadow);
#undef UPDATE_FLAGS_FOR_TOKEN
		}
	}

	Entity *SceneLoader::CreateEntity(Document::Node *n)
	{
		Document::Node *flagsNode = n->Resolve("flags");
		uint32 flags = Entity::DEFAULT_FLAGS;
		if(flagsNode != nullptr) {
			UpdateFlags(flagsNode, flags);
		}

		Entity *e = new Entity();
		e->SetFlags(flags);

		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		e->SetMatrix(pos, orientation);
		return e;
	}

	MeshEntity *SceneLoader::CreateMeshEntity(Document::Node *n) {
		const char *meshPath = n->ResolveValue("mesh.#0");
		if(meshPath == nullptr) {
			Console::Error("mesh_entity did not specify a mesh");
			return nullptr;
		}

		const char *matPath = n->ResolveValue("material.#0");
		if(matPath == nullptr) {
			Console::Error("mesh_entity did not specify a material");
			return nullptr;
		}
		
		Document::Node *flagsNode = n->Resolve("flags");
		uint32 flags = Entity::DEFAULT_FLAGS;
		if(flagsNode != nullptr) {
			UpdateFlags(flagsNode, flags);
		}

		auto eng = Engine::Get();
		MeshEntity *me = new MeshEntity(eng->assets->PathToRid(meshPath), eng->assets->PathToRid(matPath));
		me->SetFlags(flags);

		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		me->SetMatrix(pos, orientation);
		return me;
	}

	SkinnedMeshEntity *SceneLoader::CreateSkinnedMeshEntity(Document::Node *n) {
		const char *meshPath = n->ResolveValue("mesh.#0");
		if(meshPath == nullptr) {
			Console::Error("skinned_mesh_entity did not specify a mesh");
			return nullptr;
		}

		const char *matPath = n->ResolveValue("material.#0");
		if(matPath == nullptr) {
			Console::Error("skinned_mesh_entity did not specify a material");
			return nullptr;
		}

		const char *skelPath = n->ResolveValue("skeleton.#0");
		if(skelPath == nullptr) {
			Console::Error("skinned_mesh_entity did not specify a skeleton");
			return nullptr;
		}

		Document::Node *flagsNode = n->Resolve("flags");
		uint32 flags = Entity::DEFAULT_FLAGS;
		if(flagsNode != nullptr) {
			UpdateFlags(flagsNode, flags);
		}

		auto eng = Engine::Get();
		SkinnedMeshEntity *me = new SkinnedMeshEntity(eng->assets->PathToRid(meshPath), eng->assets->PathToRid(matPath), eng->assets->PathToRid(skelPath));
		me->SetFlags(flags);

		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		me->SetMatrix(pos, orientation);
		return me;
	}

	CharacterEntity *SceneLoader::CreateCharacterEntity(Document::Node *n)
	{
		Document::Node *flagsNode = n->Resolve("flags");
		uint32 flags = Entity::DEFAULT_FLAGS;
		if(flagsNode != nullptr) {
			UpdateFlags(flagsNode, flags);
		}

		CharacterEntity *ce = new CharacterEntity();
		ce->SetFlags(flags);

		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		ce->SetMatrix(pos, orientation);
		return ce;
	}

	Light *SceneLoader::CreateLight(Document::Node *n) {
		RenderState::LightType lightType;
		Document::Node *type = n->Resolve("type.#0");
		if(type != nullptr) {
			if(type->ValueEquals("directional")) {
				lightType = RenderState::LightType_Directional;
			} else if(type->ValueEquals("point")) {
				lightType = RenderState::LightType_Point;
			} else {
				Console::Error("Invalid light type: %s", type->value);
				return nullptr;
			}
		}

		Light *l = new Light(lightType);
		l->SetOn(n->ResolveAsBool("on.#0", true));
		ReadFloats(n->Resolve("diffuse"), l->properties.diffuseColor.vals, 3);
		
		Document::Node *ambientNode = n->Resolve("ambient");
		if(ambientNode != nullptr) {
			ReadFloats(ambientNode, l->properties.ambientColor.vals, 3);
		}

		Document::Node *specularNode = n->Resolve("specular");
		if(specularNode != nullptr) {
			ReadFloats(specularNode, l->properties.specularColor.vals, 3);
		}

		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Vector3 target(0.0f);
		if(ReadFloats(n->Resolve("target_pos"), target.vals, 3)) {
			Vector3 look = target - pos;
			look.Normalize();

			Vector3 lookXY = look;
			lookXY.z = 0.0f;
			lookXY.Normalize();

			Quaternion q1;
			q1.FromRotationArc(Vector3(0.0f, 0.0f, -1.0f), look);

			q1.ToEulerAngles(angles);
		}

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		l->SetMatrix(pos, orientation);


		bool shadows = n->ResolveAsBool("shadows.#0", false);
		bool splitShadows = n->ResolveAsBool("split_shadows.#0", false);
		if(shadows) {
			l->EnableShadows(n->ResolveAsUInt("shadow_map_size.#0", 1024), n->ResolveAsUInt("shadow_map_size.#1", 1024), splitShadows);
		}

		Document::Node *frustum = n->Resolve("frustum");
		if(frustum != nullptr) {
			l->fov = frustum->children[0]->ValueAsFloat();
			l->properties.widthHeightNearFar.x = frustum->children[1]->ValueAsFloat();
			l->properties.widthHeightNearFar.y = frustum->children[2]->ValueAsFloat();
			l->properties.widthHeightNearFar.z = frustum->children[3]->ValueAsFloat();
			l->properties.widthHeightNearFar.w = frustum->children[4]->ValueAsFloat();
		}

		return l;
	}

	Camera *SceneLoader::CreateCamera(Document::Node *n) {
		Camera *c = new Camera();
		Vector4 pos(0.0f);
		Vector3 angles(0.0f);
		ReadFloats(n->Resolve("pos"), pos.vals, 3);
		ReadFloats(n->Resolve("angles"), angles.vals, 3);

		Vector3 target(0.0f);
		if(ReadFloats(n->Resolve("target_pos"), target.vals, 3)) {
			Vector3 look = target - pos;
			look.Normalize();

			Vector3 lookXY = look;
			lookXY.z = 0.0f;
			lookXY.Normalize();

			Quaternion q1;
			q1.FromRotationArc(Vector3(0.0f, 0.0f, -1.0f), look);

			q1.ToEulerAngles(angles);
		}

		Quaternion orientation;
		orientation.FromEulerAngles(angles);
		c->SetMatrix(pos, orientation);

		Document::Node *frustum = n->Resolve("frustum");
		if(frustum != nullptr) {
			c->frustum.Set(
				1.0f, 1.0f,
				frustum->children[1]->ValueAsFloat(),
				frustum->children[2]->ValueAsFloat(),
				frustum->children[0]->ValueAsFloat()
				);
		}
		return c;
	}

	bool SceneLoader::ReadFloats(Document::Node *n, float *out, uint32 count) {
		if(n != nullptr) {
			if(n->count == count) {
				for(uint32 i = 0; i < count; i++) {
					out[i] = n->children[i]->ValueAsFloat();
				}
				return true;
			} else {
				Console::Warning("Expected a float vector with %d elements but it had %d, skipping", count, n->count);
			}
		}
		return false;
	}

	bool SceneLoader::ReadInts(Document::Node *n, int32 *out, uint32 count) {
		if(n != nullptr) {
			if(n->count == count) {
				for(uint32 i = 0; i < count; i++) {
					out[i] = n->children[i]->ValueAsInt();
				}
				return true;
			} else {
				Console::Warning("Expected an int vector with %d elements but it had %d, skipping", count, n->count);
			}
		}
		return false;
	}

} // namespace Maki