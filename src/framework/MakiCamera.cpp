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


} // namespace Maki