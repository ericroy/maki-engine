#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiDocument.h"
#include "core/MakiSkeleton.h"

namespace Maki
{

	Skeleton::Skeleton() : Resource() {}
	Skeleton::~Skeleton() {}

	bool Skeleton::Load(Rid rid) {
		bones.Delete();
		
		Document doc;
		if(!doc.Load(rid)) {
			Console::Error("Could not parse file as document <rid %d>", rid);
			return false;
		}

		bones.SetSize(doc.root->count);
		bones.Zero();

		Array<Joint> joints(doc.root->count);

		for(uint32 i = 0; i < bones.count; i++) {
			Document::Node *n = doc.root->children[i];
			Bone *b = &bones[i];
			Joint *j = &joints[i];
			
			int32 parentIndex = n->ResolveAsInt("#0");
			if(parentIndex >= 0) {
				assert(bones[parentIndex].childCount < MAX_CHILDREN_PER_BONE);
				bones[parentIndex].children[bones[parentIndex].childCount++] = b;
			}

			j->offset = Vector3(n->children[1]->ValueAsFloat(), n->children[2]->ValueAsFloat(), n->children[3]->ValueAsFloat());

			Vector3 eulerAngles(
				n->children[4]->ValueAsFloat() * MAKI_DEG_TO_RAD,
				n->children[5]->ValueAsFloat() * MAKI_DEG_TO_RAD,
				n->children[6]->ValueAsFloat() * MAKI_DEG_TO_RAD
			);
			j->rot.FromEulerAngles(eulerAngles);
		}

		this->rid = rid;

		inverseBindPose.Delete();
		inverseBindPose.SetSize(joints.count);
		CalculateInverseBindPose(joints.data, inverseBindPose.data);

		return true;
	}

	void Skeleton::CalculateInverseBindPose(Joint *jointStates, Matrix44 *out) {
		uint32 index = 0;
		CalculatePoseRecursive(index, Matrix44::Identity, jointStates, out);
		for(uint32 i = 0; i < bones.count; i++) {
			Matrix44::AffineInverse(out[i], out[i]);
		}
	}

	void Skeleton::CalculateWorldPose(Joint *jointStates, Matrix44 *out) {
		uint32 index = 0;
		CalculatePoseRecursive(index, Matrix44::Identity, jointStates, out);
	}

	void Skeleton::CalculatePoseRecursive(uint32 &index, const Matrix44 &current, Joint *jointStates, Matrix44 *out) {
		Matrix44 rot;
		jointStates[index].rot.ToMatrix(rot);
		Matrix44::Translation(jointStates[index].offset, rot);
		
		out[index] = current * rot;
		const Matrix44 &newCurrent = out[index];

		const Bone *b = &bones[index];
		index++;
		for(uint32 i = 0; i < b->childCount; i++) {
			CalculatePoseRecursive(index, newCurrent, jointStates, out);
		}
	}

} // namespace Maki