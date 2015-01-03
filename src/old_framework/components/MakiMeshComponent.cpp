#include "framework/framework_stdafx.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Mesh::Mesh()
				: RenderableComponent(TYPE, DEPENDENCIES),
				mesh(HANDLE_NONE),
				material(HANDLE_NONE)
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

				// Draw a quad that is 1 pixel smaller than the sprite area would suggest.
				// Shrink the uv's by a half a texel on each side.
				// These two adjustments will let us put sprites next to eachother without
				// having visible seams between them (as long as you don't use mipmaps).
				float pw = rect.GetWidth();
				float w = (pw-1) / MAKI_PPU;
				float ph = rect.GetHeight();
				float h = (ph-1) / MAKI_PPU;

				float du = 0.5f / tex->width;
				float dv = 0.5f / tex->height;

				verts[0].pos = Vector3(0.0f, 0.0f, 0.0f);
				verts[0].color = 0xffffffff;
				verts[0].uv.x = rect.left / tex->width + du;
				verts[0].uv.y = rect.top / tex->height + dv;

				verts[1].pos = Vector3(0.0f, -h, 0.0f);
				verts[1].color = 0xffffffff;
				verts[1].uv.x = rect.left / tex->width + du;
				verts[1].uv.y = (rect.top + ph) / tex->height - dv;

				verts[2].pos = Vector3(w, -h, 0.0f);
				verts[2].color = 0xffffffff;
				verts[2].uv.x = (rect.left + pw) / tex->width - du;
				verts[2].uv.y = (rect.top + ph) / tex->height - dv;

				verts[3].pos = Vector3(w, 0.0f, 0.0f);
				verts[3].color = 0xffffffff;
				verts[3].uv.x = (rect.left + pw) / tex->width - du;
				verts[3].uv.y = rect.top / tex->height + dv;

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

				c->renderFlags = renderFlags;
				
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


		} // namespace Components

	} // namespace Framework

} // namespace Maki