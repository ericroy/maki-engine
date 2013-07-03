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
				objectType(ObjectType_StaticMesh),
				mesh(HANDLE_NONE)
			{
			}

			Physics::~Physics()
			{
				MeshManager::Free(mesh);
			}

			bool Physics::Init(Document::Node *props)
			{
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
			}

			bool Physics::Init(HandleOrRid meshId)
			{
				objectType = ObjectType_StaticMesh;

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

		} // namspace Components

	} // namespace Framework

} // namespace Maki