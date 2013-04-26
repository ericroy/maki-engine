#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"


namespace Maki
{

	class DebugArmature : public Entity
	{
	private:
		struct Vertex
		{
			float pos[3];
			uint8 col[4];
		};

	public:
		DebugArmature(Rid materialRid);
		virtual ~DebugArmature();

	public:
		void Build(Skeleton *s, Skeleton::Joint *joints);
		void Update(Skeleton *s, Skeleton::Joint *joints);

	private:
		void AddLines(uint32 &index, const Matrix44 &current, Skeleton::Bone *b, Skeleton::Joint *joints);
		void AddLine(const Vector3 &from, const Vector3 &to);
		void UpdateLines(uint32 &index, const Matrix44 &current, Skeleton::Bone *b, Skeleton::Joint *joints);
		void UpdateLine(uint32 index, const Vector3 &from, const Vector3 &to);

	private:
		bool built;
		Rid materialRid;
		Handle mesh;
	};

} // namespace Maki