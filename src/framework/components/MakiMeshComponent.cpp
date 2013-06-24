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
				: Component(TYPE),
				mesh(HANDLE_NONE),
				material(HANDLE_NONE),
				scaleMatrix(true),
				meshScale(1.0f)
			{
			}

			Mesh::Mesh(HandleOrRid meshId, HandleOrRid materialId)
				: Component(TYPE),
				mesh(HANDLE_NONE),
				material(HANDLE_NONE),
				scaleMatrix(true),
				meshScale(1.0f)
			{
				bool ret = Init(meshId, materialId);
				assert(ret);
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

			bool Mesh::Init(Document::Node *node)
			{
				Engine *eng = Engine::Get();

				const char *meshPath = node->ResolveValue("mesh.#0");
				if(meshPath == nullptr) {
					Console::Error("Entity did not specify a mesh");
					return false;
				}
				Rid meshRid = eng->assets->PathToRid(meshPath);
				if(meshRid == RID_NONE) {
					Console::Error("No RID for path: %s", meshPath);
					return false;
				}

				const char *matPath = node->ResolveValue("material.#0");
				if(matPath == nullptr) {
					Console::Error("Entity did not specify a material");
					return false;
				}
				Rid matRid = eng->assets->PathToRid(matPath);
				if(matRid == RID_NONE) {
					Console::Error("No RID for path: %s", matPath);
					return false;
				}

				return Init(meshRid, matRid);
			}

			void Mesh::SetMeshScale(float scale)
			{
				scaleMatrix.SetIdentity();
				Matrix44::Scale(scale, scale, scale, scaleMatrix);
				meshScale = scale;
			}

			bool Mesh::Init(HandleOrRid meshId, HandleOrRid matId)
			{
				ResourceProvider *res = ResourceProvider::Get();

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

		} // namespace Components

	} // namespace Framework

} // namespace Maki