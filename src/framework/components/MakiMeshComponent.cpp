#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/MakiDebugArmature.h"

namespace Maki
{

	std::function<Entity *()> MeshComponent::CreateDebugArmature = nullptr;

	MeshComponent::MeshComponent()
		: Component(COMPONENT_TYPE),
		mesh(HANDLE_NONE),
		material(HANDLE_NONE),
		skeleton(HANDLE_NONE),
		scaleMatrix(true),
		meshScale(1.0f),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
	}

	MeshComponent::MeshComponent(HandleOrRid meshId, HandleOrRid materialId)
		: Component(COMPONENT_TYPE),
		mesh(HANDLE_NONE),
		material(HANDLE_NONE),
		skeleton(HANDLE_NONE),
		scaleMatrix(true),
		meshScale(1.0f),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
		bool ret = Init(meshId, materialId, HandleOrRid());
		assert(ret);
	}

	MeshComponent::MeshComponent(HandleOrRid meshId, HandleOrRid materialId, HandleOrRid skeletonId)
		: Component(COMPONENT_TYPE),
		mesh(HANDLE_NONE),
		material(HANDLE_NONE),
		skeleton(HANDLE_NONE),
		scaleMatrix(true),
		meshScale(1.0f),
		armature(nullptr),
		materialSlot(-1),
		poseDirty(true)
	{
		bool ret = Init(meshId, materialId, skeletonId);
		assert(ret);
	}

	MeshComponent::~MeshComponent()
	{
		const uint32 count = drawCommands.count;
		for(uint32 i = 0; i < count; i++) {
			drawCommands[i].~DrawCommand();
		}

		MeshManager::Free(mesh);
		MaterialManager::Free(material);
		SkeletonManager::Free(skeleton);
	}

	bool MeshComponent::Init(Document::Node *node)
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

		Rid skelRid = RID_NONE;
		const char *skelPath = node->ResolveValue("skeleton.#0");
		if(skelPath != nullptr) {
			skelRid = Engine::Get()->assets->PathToRid(skelPath);
			if(skelRid == RID_NONE) {
				Console::Error("No RID for path: %s", skelRid);
				return false;
			}
		}

		return Init(meshRid, matRid, skelRid);
	}

	void MeshComponent::SetMeshScale(float scale)
	{
		scaleMatrix.SetIdentity();
		Matrix44::Scale(scale, scale, scale, scaleMatrix);
		owner->bounds.radii *= scale / meshScale;
		meshScale = scale;
	}

	bool MeshComponent::Init(HandleOrRid meshId, HandleOrRid matId, HandleOrRid skelId)
	{
		ResourceProvider *res = ResourceProvider::Get();

		owner->SetFlag(Entity::Flag_Draw);

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

		if(skelId.isHandle && skelId.handle != HANDLE_NONE) {
			SkeletonManager::AddRef(skelId.handle);
			skeleton = skelId.handle;
		} else if(skelId.rid != RID_NONE) {
			skeleton = res->skeletonManager->Load(skelId.rid);
			assert(skeleton != HANDLE_NONE);
		}


		if(skeleton != HANDLE_NONE) {
			owner->SetFlag(Entity::Flag_Update);

			// Skinned meshes need exclusive access to their material (since they must write matrices to it)
			material = MaterialManager::DuplicateIfShared(material);

			// Make our buffers the right size for this skeleton
			Skeleton *skel = SkeletonManager::Get(skeleton);
			pose.SetSize(skel->bones.count);
			matrixPose.SetSize(skel->bones.count);

			// Setup the material constant slot that will receive the bone matrices
			// Bone matrices are 3x4 = 12 floats = 12*4 bytes
			Material *mat = MaterialManager::Get(material);
			char *buffer = (char *)Allocator::Malloc(skel->bones.count*12*4);
			materialSlot = mat->PushConstant("uBoneStates", skel->bones.count*12*4, buffer);
		
			if(CreateDebugArmature != nullptr) {
				armature = (DebugArmature *)CreateDebugArmature();
				owner->AddChild((Entity *)armature);
			}
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

		if(skeleton != HANDLE_NONE) {
			updateFunc = [&](float dt) {
				if(!poseDirty) {
					return;
				}
				poseDirty = false;

				Skeleton *skel = SkeletonManager::Get(skeleton);

				skel->CalculateWorldPose(pose.data, matrixPose.data);
				if(armature != nullptr) {
					armature->Update(skel, pose.data);
				}

				// Combine inverse bind and world matrices
				// Store them in the material constant buffer (as 3x4 matrices)
				Material *mat = MaterialManager::Get(material);
				float *boneMatrices = (float *)mat->uniformValues[materialSlot].data;

				Matrix44 temp;
				for(uint32 i = 0; i < pose.count; i++) {
					temp = matrixPose[i] * skel->inverseBindPose[i];
					const uint32 base = i*12;
					boneMatrices[base] = temp.cols[0][0];
					boneMatrices[base+1] = temp.cols[0][1];
					boneMatrices[base+2] = temp.cols[0][2];
					boneMatrices[base+3] = temp.cols[1][0];
					boneMatrices[base+4] = temp.cols[1][1];
					boneMatrices[base+5] = temp.cols[1][2];
					boneMatrices[base+6] = temp.cols[2][0];
					boneMatrices[base+7] = temp.cols[2][1];
					boneMatrices[base+8] = temp.cols[2][2];
					boneMatrices[base+9] = temp.cols[3][0];
					boneMatrices[base+10] = temp.cols[3][1];
					boneMatrices[base+11] = temp.cols[3][2];
				}
			};
		}

		// Merge our mesh's bounding box with that of the owner
		owner->bounds.Merge(BoundingBox(m->bounds.pos, m->bounds.radii));

		return true;
	}


} // namespace Maki