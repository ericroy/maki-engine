#include "framework/framework_stdafx.h"
#include "framework/MakiCamera.h"

namespace Maki
{

	std::function<Entity *()> Camera::CreateDebugWidget = nullptr;

	Camera::Camera()
		: Entity(Flag_Update|Flag_ProcessChildren),
		frustum(0.0f, 0.0f, 0.0f, 1.0f, 50.0f)
	{
#if _DEBUG
		if(CreateDebugWidget != nullptr) {
			Entity *debugWidget = CreateDebugWidget();
			if(debugWidget != nullptr) {
				AddChild(debugWidget);
			}
		}
#endif
	}

	Camera::~Camera()
	{
	}






	CameraFactory::CameraFactory()
		: EntityFactory(), hasFrustum(false), hasTarget(false), target(0.0f)
	{
	}

	CameraFactory::~CameraFactory()
	{
	}

	bool CameraFactory::PreCreate(Document::Node *node)
	{
		if(!EntityFactory::PreCreate(node)) {
			return false;
		}

		if(node->ResolveAsVectorN("target_pos", 3, target.vals)) {
			hasTarget = true;
		}

		Document::Node *frustumNode = node->Resolve("frustum");
		if(frustumNode != nullptr) {
			hasFrustum = true;
			frustum.Set(
				1.0f, 1.0f,
				frustumNode->children[1]->ValueAsFloat(),
				frustumNode->children[2]->ValueAsFloat(),
				frustumNode->children[0]->ValueAsFloat()
			);
		}

		return true;
	}

	Camera *CameraFactory::Create()
	{
		return new Camera();
	}

	void CameraFactory::PostCreate(Camera *cam)
	{
		EntityFactory::PostCreate(cam);

		if(hasTarget) {
			Vector3 look = target - pos;
			look.Normalize();

			Vector3 lookXY = look;
			lookXY.z = 0.0f;
			lookXY.Normalize();

			Quaternion q1;
			q1.FromRotationArc(Vector3(0.0f, 0.0f, -1.0f), look);

			q1.ToEulerAngles(angles);
			cam->SetOrientation(Quaternion(angles));
		}

		if(hasFrustum) {
			cam->frustum = frustum;
		}
	}


} // namespace Maki