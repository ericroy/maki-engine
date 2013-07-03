#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiPhysicsComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Physics::Physics()
				: Component(TYPE, DEPENDENCIES),
				objectType(ObjectType_Static),
				objectShape(ObjectShape_Mesh),
				mesh(HANDLE_NONE)
			{
			}

			Physics::~Physics()
			{
				if(objectShape == ObjectShape_Mesh) {
					MeshManager::Free(mesh);
				}
			}

			bool Physics::Init(Document::Node *props)
			{
				Document::Node *typeNode = props->Resolve("type.#0");

				if(typeNode->ValueEquals("box")) {
					
					Vector4 minCorner(0.0f), maxCorner(0.0f);
					if(!props->ResolveAsVectorN("min", 3, minCorner.vals)) {
						Console::Error("Physics box needs 'min' node");
						return false;
					}
					if(!props->ResolveAsVectorN("max", 3, maxCorner.vals)) {
						Console::Error("Physics box needs 'max' node");
						return false;
					}

					return Init(minCorner, maxCorner);

				} else if(typeNode->ValueEquals("mesh")) {

					const char *meshPath = props->ResolveValue("mesh.#0");
					if(meshPath == nullptr) {
						Console::Error("Entity did not specify a mesh");
						return false;
					}
					Rid meshRid = Engine::Get()->assets->PathToRid(meshPath);
					if(meshRid == RID_NONE) {
						Console::Error("No RID for path: %s", meshPath);
						return false;
					}

					return Init(meshRid);

				} else {
					Console::Error("Unrecognized physics component type: %s", typeNode->value);
					return false;
				}
			}

			bool Physics::Init(HandleOrRid meshId)
			{
				objectType = ObjectType_Static;
				objectShape = ObjectShape_Mesh;

				if(meshId.isHandle) {
					assert(meshId.handle != HANDLE_NONE);
					MeshManager::AddRef(meshId.handle);
					mesh = meshId.handle;
				} else {
					mesh = CoreManagers::Get()->meshManager->Load(meshId.rid);
					assert(mesh != HANDLE_NONE);
				}

				return true;
			}

			bool Physics::Init(const Vector4 &minCorner, const Vector4 &maxCorner)
			{
				objectType = ObjectType_Dynamic;
				objectShape = ObjectShape_Box;

				this->minCorner = minCorner;
				this->maxCorner = maxCorner;

				return true;
			}

		} // namspace Components

	} // namespace Framework

} // namespace Maki