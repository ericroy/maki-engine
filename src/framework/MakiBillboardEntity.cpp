#include "framework/framework_stdafx.h"
#include "framework/MakiBillboardEntity.h"

namespace Maki
{

	BillboardEntity::BillboardEntity(HandleOrRid matId, HandleOrRid texId)
		: Entity(), mode(BillboardMode_None), facingAxis(Vector4::UnitX), constraintAxis(Vector4::UnitZ)
	{
		bool ret = Init(matId, texId);
		assert(ret);
	}

	BillboardEntity::BillboardEntity(HandleOrRid matId, HandleOrRid texId, BillboardMode mode, const Vector4 &facingAxis, const Vector4 &constraintAxis)
		: Entity(), mode(mode), facingAxis(facingAxis), constraintAxis(constraintAxis)
	{
		bool ret = Init(matId, texId);
		assert(ret);
	}

	BillboardEntity::~BillboardEntity()
	{
		MeshManager::Free(mesh);
		MaterialManager::Free(material);
	}

	bool BillboardEntity::Init(HandleOrRid matId, HandleOrRid texId)
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

		material = res->materialManager->DuplicateIfShared(material);

		Rid texRid = RID_NONE;
		Handle texture = HANDLE_NONE;
		if(texId.isHandle) {
			if(texId.handle != HANDLE_NONE) {
				TextureManager::AddRef(matId.handle);
				texture = texId.handle;
				texRid = TextureManager::Get(texture)->rid;
				assert(texRid != RID_NONE);
			}
		} else {
			texRid = texId.rid;
			texture = res->textureManager->Load(texId.rid);
		}

		Texture *tex = TextureManager::Get(texture);
		const float ppm = 1024.0f;
		float width = (float)tex->width / ppm;
		float height = (float)tex->height / ppm;

		TextureSet ts;
		ts.textureRids[ts.textureCount] = texRid;
		ts.textures[ts.textureCount++] = texture;
		
		Handle textureSet = res->textureSetManager->Find(ts);
		if(textureSet != HANDLE_NONE) {
			TextureManager::Free(texture);
		} else {
			textureSet = res->textureSetManager->Add(Move(ts));
		}

		Material *mat = res->materialManager->Get(material);
		TextureSetManager::Free(mat->textureSet);
		mat->textureSet = textureSet;

		
		Mesh m;
		m.SetVertexAttributes(VertexFormat::AttributeFlag_Color|VertexFormat::AttributeFlag_TexCoord);
		m.SetIndexAttributes(3, 2);
		struct V {
			float pos[3];
			uint8 col[4];
			float uv[2];
		};
		float left = -width/2.0f;
		float right = width/2.0f;
		float bottom = 0.0f;
		float top = height;
		V v[4] = {
			{0, left, bottom, 255, 255, 255, 255, 0, 1},
			{0, left, top, 255, 255, 255, 255, 0, 0},
			{0, right, top, 255, 255, 255, 255, 1, 0},
			{0, right, bottom, 255, 255, 255, 255, 1, 1},
		};
		m.PushVertexData(sizeof(v), (char *)v);
		uint16 f[6] = {0, 2, 1, 0, 3, 2};
		m.PushIndexData(sizeof(f), (char *)f);
		m.CalculateBounds();
		m.Upload();

		bounds.Merge(BoundingBox(m.bounds.pos, m.bounds.radii));
		mesh = res->meshManager->Add(Move(m));

		// Allocate and initialize draw commands
		dc.SetMaterial(material);
		dc.SetMesh(mesh);

		return true;
	}

	void BillboardEntity::Draw(Renderer *renderer) {
		if(mode == BillboardMode_None) {
			renderer->Draw(dc, world);
			return;
		}

		if(mode == BillboardMode_Face) {
			matrix = renderer->GetCameraMatrix();
			Matrix44::Translation(position.x, position.y, position.z, matrix);
			UpdateWorldMatrix();
			renderer->Draw(dc, world);
		
		} else {
			Vector4 camLookDir = renderer->GetCameraMatrix() * Vector4(0.0f, 0.0f, -1.0f, 0.0f);
			camLookDir.w = 1.0f;
			float d = camLookDir.Dot(constraintAxis);

			// Skip colinear case
			if(fabs(d) < 0.999995) {
				// Eliminate look component along pivot axis
				camLookDir -= constraintAxis * d;

				orientation.FromRotationArc(facingAxis, -camLookDir);
				UpdateMatrix();
				UpdateWorldMatrix();
			}
			renderer->Draw(dc, world);
		}
	}








	BillboardEntityFactory::BillboardEntityFactory()
		: EntityFactory(), textureRid(RID_NONE), matRid(RID_NONE), mode(BillboardEntity::BillboardMode_None), facing(Vector4::UnitX), constraint(Vector4::UnitZ)
	{
	}

	BillboardEntityFactory::~BillboardEntityFactory()
	{
	}

	bool BillboardEntityFactory::PreCreate(Document::Node *node)
	{
		if(!EntityFactory::PreCreate(node)) {
			return false;
		}

		const char *matPath = node->ResolveValue("material.#0");
		if(matPath == nullptr) {
			Console::Error("Entity did not specify a material");
			return false;
		}
		matRid = Engine::Get()->assets->PathToRid(matPath);
		if(matRid == RID_NONE) {
			Console::Error("No RID for path: %s", matRid);
			return false;
		}

		const char *texturePath = node->ResolveValue("texture.#0");
		if(texturePath == nullptr) {
			Console::Error("Entity did not specify a texture");
			return false;
		}
		textureRid = Engine::Get()->assets->PathToRid(texturePath);
		if(textureRid == RID_NONE) {
			Console::Error("No RID for path: %s", texturePath);
			return false;
		}

		Document::Node *modeNode = node->Resolve("mode.#0");
		if(modeNode != nullptr) {
			if(modeNode->ValueEquals("face")) {
				mode = BillboardEntity::BillboardMode_Face;
			} else if(modeNode->ValueEquals("pivot")) {
				mode = BillboardEntity::BillboardMode_Pivot;
			} else if(modeNode->ValueEquals("none")) {
				mode = BillboardEntity::BillboardMode_None;
			} else {
				Console::Error("Invalid billboard mode: %s", modeNode->value);
				return false;
			}
		}

		if(mode == BillboardEntity::BillboardMode_Face) {
			node->ResolveAsVectorN("facing", 3, facing.vals);
		} else if(mode ==BillboardEntity::BillboardMode_Pivot) {
			node->ResolveAsVectorN("facing", 3, facing.vals);
			node->ResolveAsVectorN("constraint", 3, constraint.vals);
		}

		return true;
	}

	BillboardEntity *BillboardEntityFactory::Create()
	{
		return new BillboardEntity(matRid, textureRid, mode, facing, constraint);
	}

	void BillboardEntityFactory::PostCreate(BillboardEntity *e)
	{
		EntityFactory::PostCreate(e);
	}

} // namespace Maki