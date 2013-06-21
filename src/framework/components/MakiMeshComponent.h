#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class MeshComponent : public Component
	{
	public:
		MeshComponent()
			: Component(Type_Mesh), mesh(HANDLE_NONE), material(HANDLE_NONE), scaleMatrix(true), meshScale(1.0f)
		{
		}

		MeshComponent(HandleOrRid meshId, HandleOrRid materialId)
			: Component(Type_Mesh), mesh(HANDLE_NONE), material(HANDLE_NONE), scaleMatrix(true), meshScale(1.0f)
		{
			bool ret = Init(meshId, materialId);
			assert(ret);
		}

		virtual ~MeshComponent()
		{
		}

		virtual bool Init(Document::Node *node)
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

		virtual void Attach(Entity *entity)
		{
			Component::Attach(entity);

			// Merge our mesh's bounding box with that of the owner
			const Mesh *m = MeshManager::Get(mesh);
			owner->bounds.Merge(BoundingBox(m->bounds.pos, m->bounds.radii));
		}

		void SetMeshScale(float scale)
		{
			scaleMatrix.SetIdentity();
			Matrix44::Scale(scale, scale, scale, scaleMatrix);
			owner->bounds.radii *= scale / meshScale;
			meshScale = scale;
		}

	private:
		bool Init(HandleOrRid meshId, HandleOrRid matId)
		{
			ResourceProvider *res = ResourceProvider::Get();
		
			if(matId.isHandle) {
				if(matId.handle != HANDLE_NONE) {
					MaterialManager::AddRef(matId.handle);
					material = matId.handle;
				}
			} else {
				material = res->materialManager->Load(matId.rid);
			}

			if(meshId.isHandle) {
				assert(meshId.handle != HANDLE_NONE);
				MeshManager::AddRef(meshId.handle);
				mesh = meshId.handle;
			} else {
				mesh = res->meshManager->Load(meshId.rid);
				assert(mesh != HANDLE_NONE);
			}
		
			// Allocate and initialize draw commands
			const Mesh *m = MeshManager::Get(mesh);
			const uint32 count = m->siblings.size()+1;
			drawCommands.SetSize(count);
			for(uint32 i = 0; i < count; i++) {
				DrawCommand *dc = &drawCommands[i];
				new(dc) DrawCommand();
				dc->SetMesh(i == 0 ? mesh : m->siblings[i-1]);
				dc->SetMaterial(material);
			}


			drawFunc = [&](Renderer *renderer) {
				Engine *eng = Engine::Get();

				const uint32 count = drawCommands.count;
				for(uint32 i = 0; i < count; i++) {
#if _DEBUG
					// I don't want to set this every draw call for efficiency reasons, but if we don't
					// then hot swapping materials doesn't have any effect.  Perhaps we'll just leave this on
					// in debug mode for now
					drawCommands[i].SetMaterial(material);
#endif
					renderer->Draw(drawCommands[i], owner->GetWorldMatrix() * scaleMatrix);
				}
			};

			return true;
		}

	public:
		Handle mesh;
		Handle material;
		Array<DrawCommand> drawCommands;

	private:
		float meshScale;
		Matrix44 scaleMatrix;
	};


} // namespace Maki