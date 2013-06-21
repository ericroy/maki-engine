#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{

	class CameraComponent : public Component
	{
	public:
		static const Type COMPONENT_TYPE = Type::Type_Camera;
		static std::function<Entity *()> CreateDebugWidget;

	public:
		CameraComponent();
		virtual ~CameraComponent();
		virtual bool Init(Document::Node *node);
		virtual void Attach(Entity *owner);

	public:
		Frustum frustum;

		// Only for init from maki-doc...  what to do about this cruft?
		bool hasTarget;
		bool hasFrustum;
	};

}