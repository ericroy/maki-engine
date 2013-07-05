#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiMeshComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Mesh::Mesh()
				: Component(TYPE, DEPENDENCIES),
				flags(DEFAULT_FLAGS),
				mesh(HANDLE_NONE),
				material(HANDLE_NONE),
				scaleMatrix(true),
				meshScale(1.0f)
			{
			}

			Mesh::~Mesh()
			{
				const uint32 count = drawCommands.count;
				for(uint32 i = 0; i < count; i++) {
					drawCommands[i].~DrawCommand();
				}

				MeshManager::Free(mesh);
				MaterialManager::Free(material);
			}

			bool Mesh::Init(Document::Node *props)
			{
				Engine *eng = Engine::Get();

				const char *meshPath = props->ResolveValue("mesh.#0");
				if(meshPath == nullptr) {
					Console::Error("Mesh component did not specify a mesh");
					return false;
				}
				Rid meshRid = eng->assets->PathToRid(meshPath);
				if(meshRid == RID_NONE) {
					Console::Error("No RID for path: %s", meshPath);
					return false;
				}

				const char *matPath = props->ResolveValue("material.#0");
				if(matPath == nullptr) {
					Console::Error("Mesh component did not specify a material");
					return false;
				}
				Rid matRid = eng->assets->PathToRid(matPath);
				if(matRid == RID_NONE) {
					Console::Error("No RID for path: %s", matPath);
					return false;
				}

				return Init(meshRid, matRid);
			}

			bool Mesh::Init(HandleOrRid meshId, HandleOrRid matId)
			{
				CoreManagers *res = CoreManagers::Get();

				if(meshId.isHandle) {
					assert(meshId.handle != HANDLE_NONE);
					MeshManager::AddRef(meshId.handle);
					mesh = meshId.handle;
				} else {
					mesh = res->meshManager->Load(meshId.rid);
					assert(mesh != HANDLE_NONE);
				}

				if(matId.isHandle && matId.handle != HANDLE_NONE) {
					MaterialManager::AddRef(matId.handle);
					material = matId.handle;
				} else if(matId.rid != RID_NONE) {
					material = res->materialManager->Load(matId.rid);
				}	
		
				// Allocate and initialize draw commands
				const Core::Mesh *m = MeshManager::Get(mesh);
				const uint32 count = m->siblings.size()+1;
				drawCommands.SetSize(count);
				for(uint32 i = 0; i < count; i++) {
					DrawCommand *dc = &drawCommands[i];
					new(dc) DrawCommand();
					dc->SetMesh(i == 0 ? mesh : m->siblings[i-1]);
					dc->SetMaterial(material);
				}

				bounds.Merge(m->bounds);
				return true;
			}

			void Mesh::SetMeshScale(float scale)
			{
				scaleMatrix.SetIdentity();
				Matrix44::Scale(scale, scale, scale, scaleMatrix);
				meshScale = scale;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki