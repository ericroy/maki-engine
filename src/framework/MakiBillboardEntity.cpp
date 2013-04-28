#include "framework/framework_stdafx.h"
#include "framework/MakiBillboardEntity.h"

namespace Maki
{

	BillboardEntity::BillboardEntity(HandleOrRid matId, HandleOrRid texId)
		: Entity(), mode(BillboardMode_None), facingAxis(Vector4::UnitX), pivotAxis(Vector4::UnitZ)
	{
		bool ret = Init(matId, texId);
		assert(ret);
	}

	BillboardEntity::BillboardEntity(HandleOrRid matId, HandleOrRid texId, BillboardMode mode, const Vector4 &facingAxis, const Vector4 &pivotAxis)
		: Entity(), mode(mode), facingAxis(facingAxis), pivotAxis(pivotAxis)
	{
		bool ret = Init(matId, texId);
		assert(ret);
	}

	BillboardEntity::~BillboardEntity()
	{
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
		float width = (float)tex->width;
		float height = (float)tex->height;

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

		Mesh::RectArgs args;
		args.facingAxis = Vector4::UnitX;
		args.left = -width/2.0f;
		args.right = width/2.0f;
		args.bottom = 0.0f;
		args.top = height;
		Mesh m(Mesh::Object_Rect, args);
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
			
		Vector4 basisWorldPos(world.cols[3]);
		Vector4 toCamera = -Vector4(renderer->GetView().cols[3]) - basisWorldPos;
		toCamera.Normalize();

		if(mode == BillboardMode_Face) {
			orientation = Quaternion::Billboard(toCamera, facingAxis);
		} else {
			orientation = Quaternion::Billboard(toCamera, facingAxis, pivotAxis);
		}
		UpdateMatrix();
		UpdateWorldMatrix();

		renderer->Draw(dc, world);
	}








	BillboardEntityFactory::BillboardEntityFactory()
		: EntityFactory(), textureRid(RID_NONE), matRid(RID_NONE), mode(BillboardEntity::BillboardMode_None), facing(Vector4::UnitX), pivot(Vector4::UnitZ)
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
			Console::Error("No RID for path: %s", textureRid);
			return false;
		}

		Document::Node *modeNode = node->Resolve("mode");
		if(modeNode != nullptr) {
			if(modeNode->ValueEquals("face")) {
				mode = BillboardEntity::BillboardMode_Face;
			} else if(modeNode->ValueEquals("pivot")) {
				mode = BillboardEntity::BillboardMode_Pivot;
			} else {
				Console::Error("Invalid billboard mode: %s", modeNode->value);
				return false;
			}
		}

		if(mode == BillboardEntity::BillboardMode_Face) {
			node->ResolveAsVectorN("facing", 3, facing.vals);
		} else if(mode ==BillboardEntity::BillboardMode_Pivot) {
			node->ResolveAsVectorN("facing", 3, facing.vals);
			node->ResolveAsVectorN("pivot", 3, pivot.vals);
		}

		return true;
	}

	BillboardEntity *BillboardEntityFactory::Create()
	{
		return new BillboardEntity(matRid, textureRid, mode, facing, pivot);
	}

	void BillboardEntityFactory::PostCreate(BillboardEntity *e)
	{
		EntityFactory::PostCreate(e);
	}

} // namespace Maki