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

		hasTarget = node->ResolveAsVectorN("target", 3, target.vals);

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
			Matrix44 lookAt;
			Matrix44::LookAt(pos, target, Vector4::UnitZ, lookAt);
			Matrix44::Inverse(lookAt, lookAt);
			cam->SetWorldMatrix(lookAt);
		}

		if(hasFrustum) {
			cam->frustum = frustum;
		}
	}


} // namespace Maki