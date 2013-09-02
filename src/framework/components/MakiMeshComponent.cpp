#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			const float PPU = 150.0f;

			Vector2 unitQuadCoeffs[4] = {
				Vector2(0.0f, 0.0f),
				Vector2(0.0f, -1.0f),
				Vector2(1.0f, -1.0f),
				Vector2(1.0f, 0.0f)
			};

			Vector2 unitQuadTexRectCoeff[4] = {
				Vector2(0.0f, 0.0f),
				Vector2(0.0f, 1.0f),
				Vector2(1.0f, 1.0f),
				Vector2(1.0f, 0.0f)
			};


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

				Document::Node *spriteNode = props->Resolve("sprite");
				if(spriteNode != nullptr) {
					
					const char *texPath = spriteNode->ResolveValue("texture.#0");
					if(texPath == nullptr) {
						Console::Error("Mesh component (sprite) did not specify a texture");
						return false;
					}
					Rid texRid = eng->assets->PathToRid(texPath);
					if(texRid == RID_NONE) {
						Console::Error("No RID for path: %s", texPath);
						return false;
					}

					float r[4];
					spriteNode->ResolveAsVectorN("rect", 4, r);

					return InitSprite(matRid, texRid, Rect(r[0], r[0]+r[2], r[1], r[1]+r[3]));

				} else {
					
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
					
					return Init(meshRid, matRid);
				}
			}

			bool Mesh::InitSprite(HandleOrRid matId, HandleOrRid texId, const Rect &rect)
			{
				CoreManagers *res = CoreManagers::Get();

				if(matId.isHandle && matId.handle != HANDLE_NONE) {
					MaterialManager::AddRef(matId.handle);
					material = matId.handle;
				} else if(matId.rid != RID_NONE) {
					material = res->materialManager->Load(matId.rid);
				}

				Texture *tex = nullptr;
				if((texId.isHandle && texId.handle != HANDLE_NONE) || (!texId.isHandle && texId.rid != RID_NONE)) {
					
					// Get exclusive access to material
					material = MaterialManager::DuplicateIfShared(material);
					Material *mat = MaterialManager::Get(material);

					// Create a new texture set for this material
					TextureSetManager::Free(mat->textureSet);
					if(texId.isHandle) {
						tex = TextureManager::Get(texId.handle);
						TextureManager::AddRef(texId.handle);
						TextureSet ts;
						ts.textures[ts.textureCount] = texId.handle;
						ts.textures[ts.textureCount++] = tex->rid;
						mat->textureSet = res->textureSetManager->Add(Move(ts));
					} else {
						mat->SetTextures(1, &texId.rid);
						tex = TextureManager::Get(TextureSetManager::Get(mat->textureSet)->textures[0]);
					}
				} else {
					Material *mat = MaterialManager::Get(material);
					if(mat->textureSet != HANDLE_NONE) {
						TextureSet *ts = TextureSetManager::Get(mat->textureSet);
						if(ts->textureCount > 0) {
							tex = TextureManager::Get(ts->textures[0]);
						}
					}
				}

				if(tex == nullptr) {
					Console::Error("Sprite cannot be initialized without a texture (it has to know texture dimensions)");
					return false;
				}

				struct V
				{
					Vector3 pos;
					uint32 color;
					Vector2 uv;
				} verts[4];

				Core::Mesh m;
				m.SetIndexAttributes(3, 2);
				m.SetVertexAttributes(VertexFormat::AttributeFlag_Color|VertexFormat::AttributeFlag_TexCoord);
				m.SetMeshFlag(Core::Mesh::MeshFlag_HasTranslucency);

				float w = rect.GetWidth();
				float h = rect.GetHeight();

				for(int32 i = 0; i < 4; i++) {
					verts[i].pos = Vector3(unitQuadCoeffs[i].x * w / PPU, unitQuadCoeffs[i].y * h / PPU, 0);
					verts[i].color = 0xffffffff;
					verts[i].uv.x = (rect.left + unitQuadTexRectCoeff[i].x * w) / tex->width;
					verts[i].uv.y = (rect.top + unitQuadTexRectCoeff[i].y * h) / tex->height;
				}
				m.PushVertexData(sizeof(verts), (char *)verts);

				uint16 indices[6] = { 0, 1, 2, 0, 2, 3 };
				m.PushIndexData(sizeof(indices), (char *)indices);

				m.Upload();
				bounds.Merge(m.bounds);
		
				mesh = res->meshManager->Add(Move(m));

				// Allocate and initialize draw commands
				drawCommands.SetSize(1);
				DrawCommand *dc = &drawCommands[0];
				new(dc) DrawCommand();
				dc->SetMesh(mesh);
				dc->SetMaterial(material);
				
				return true;
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

			Mesh *Mesh::Clone(bool prototype)
			{
				Mesh *c = ComponentPool<Mesh>::Get()->Create();

				c->flags = flags;
				
				MeshManager::AddRef(mesh);
				c->mesh = mesh;

				MaterialManager::AddRef(material);
				c->material = material;
				
				c->drawCommands.SetSize(drawCommands.count);
				for(uint32 i = 0; i < drawCommands.count; i++) {
					DrawCommand *dc = &c->drawCommands[i];
					new(dc) DrawCommand();
					dc->Copy(drawCommands[i]);
				}
				
				c->meshScale = meshScale;
				c->scaleMatrix = scaleMatrix;
				c->bounds = bounds;
				return c;
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