#include "framework/framework_stdafx.h"
#include "framework/MakiDebugArmature.h"
#include "framework/MakiMeshEntity.h"

namespace Maki
{

	DebugArmature::DebugArmature(Rid materialRid)
	:	Entity(),
		mesh(HANDLE_NONE),
		materialRid(materialRid),
		built(false)
	{
		Mesh m(true);
		m.SetVertexAttributes(VertexFormat::AttributeFlag_Color);
		m.SetIndexAttributes(2, 2);
		mesh = ResourceProvider::Get()->meshManager->Add(Move(m));
	}

	DebugArmature::~DebugArmature() {
		MeshManager::Free(mesh);
	}
	
	void DebugArmature::AddLine(const Vector3 &from, const Vector3 &to) {
		Mesh *m = MeshManager::Get(mesh);
		Vertex v[2] = {
			{from.x, from.y, from.z, 255, 255, 20, 255},
			{to.x, to.y, to.z, 255, 20, 255, 255}
		};
		m->PushVertexData(sizeof(v), (char *)v);
		
		uint32 vertCount = m->GetVertexCount();
		uint16 f[2] = {vertCount-2, vertCount-1};
		m->PushIndexData(sizeof(f), (char *)f);
	}
	
	void DebugArmature::UpdateLine(uint32 index, const Vector3 &from, const Vector3 &to) {
		Mesh *m = MeshManager::Get(mesh);
		Vertex *data = (Vertex *)m->GetVertexData();
		Vertex &a = data[index*2];
		memcpy(a.pos, from.vals, sizeof(float)*3);
		Vertex &b = data[index*2+1];
		memcpy(b.pos, to.vals, sizeof(float)*3);
	}

	void DebugArmature::AddLines(uint32 &index, const Matrix44 &current, Skeleton::Bone *b, Skeleton::Joint *joints) {
		Vector4 from = current * Vector4(0.0);
		Vector3 off = joints[index].offset;
		Vector4 to = current * Vector4(off.x, off.y, off.z, 1.0f);
		AddLine(from, to);

		Matrix44 xform;
		joints[index].rot.ToMatrix(xform);
		Matrix44::Translation(joints[index].offset, xform);
		Matrix44 newCurrent = current * xform;
				
		index++;
		for(uint32 i = 0; i < b->childCount; i++) {
			AddLines(index, newCurrent, b->children[i], joints);
		}
	}
	
	void DebugArmature::UpdateLines(uint32 &index, const Matrix44 &current, Skeleton::Bone *b, Skeleton::Joint *joints) {
		Vector4 from = current * Vector4(0.0);
		Vector3 off = joints[index].offset;
		Vector4 to = current * Vector4(off.x, off.y, off.z, 1.0f);
		UpdateLine(index, from, to);
		
		Matrix44 xform;
		joints[index].rot.ToMatrix(xform);
		Matrix44::Translation(joints[index].offset, xform);
		Matrix44 newCurrent = current * xform;
				
		index++;
		for(uint32 i = 0; i < b->childCount; i++) {
			UpdateLines(index, newCurrent, b->children[i], joints);
		}
	}

	void DebugArmature::Build(Skeleton *s, Skeleton::Joint *joints) {
		if(!built) {
			uint32 index = 0;
			AddLines(index, Matrix44::Identity, &s->bones[0], joints);
			Mesh *m = MeshManager::Get(mesh);
			m->CalculateBounds();
			m->Upload();

			MeshEntity *me = new MeshEntity(mesh, materialRid);
			me->SetFlag(Entity::Flag_CastShadow, false);
			AddChild(me);
			built = true;
		}
	}

	void DebugArmature::Update(Skeleton *s, Skeleton::Joint *joints) {
		if(!built) {
			Build(s, joints);
		} else {
			uint32 index = 0;
			UpdateLines(index, Matrix44::Identity, &s->bones[0], joints);
			Mesh *m = MeshManager::Get(mesh);
			m->CalculateBounds();
			m->Upload();
		}
	}

} // namespace Maki