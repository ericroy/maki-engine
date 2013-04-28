#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	class Camera : public Entity
	{
	public:
		static std::function<Entity *()> CreateDebugWidget;

	public:
		Camera();
		virtual ~Camera();

	public:
		Frustum frustum;
	};



	class CameraFactory : private EntityFactory
	{
	public:
		CameraFactory();
		virtual ~CameraFactory();
		virtual bool PreCreate(Document::Node *node);
		Camera *Create();
		virtual void PostCreate(Camera *cam);

	protected:
		bool hasTarget;
		Vector4 target;

		bool hasFrustum;
		Frustum frustum;
	};

} // namespace Maki