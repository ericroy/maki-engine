#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiNavMeshComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			NavMesh::NavMesh()
				: Component(TYPE, DEPENDENCIES),
				mesh(HANDLE_NONE)
			{
			}

			NavMesh::~NavMesh()
			{
				MeshManager::Free(mesh);
			}

			bool NavMesh::Init(Document::Node *props)
			{
				Engine *eng = Engine::Get();

				const char *meshPath = props->ResolveValue("mesh.#0");
				if(meshPath == nullptr) {
					Console::Error("NavMesh component did not specify a mesh");
					return false;
				}
				Rid meshRid = eng->assets->PathToRid(meshPath);
				if(meshRid == RID_NONE) {
					Console::Error("No RID for path: %s", meshPath);
					return false;
				}

				return Init(meshRid);
			}

			bool NavMesh::Init(HandleOrRid meshId)
			{
				CoreManagers *res = CoreManagers::Get();

				if(meshId.isHandle) {
					if(meshId.handle == HANDLE_NONE) {
						Console::Error("NavMesh mesh handle cannot be HANDLE_NONE");
						return false;
					}
					MeshManager::AddRef(meshId.handle);
					mesh = meshId.handle;
				} else {
					mesh = res->meshManager->Load(meshId.rid);
					if(meshId.handle == HANDLE_NONE) {
						Console::Error("NavMesh failed to load mesh with Rid<%u>", meshId.rid);
						return false;
					}
				}

				return true;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki