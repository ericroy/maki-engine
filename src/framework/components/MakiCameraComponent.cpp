#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			std::function<Entity *()> Camera::CreateDebugWidget = nullptr;


			Camera::Camera()
				: Component(TYPE), frustum(0.0f, 0.0f, 0.0f, 1.0f, 50.0f)
			{
			}
		
			Camera::~Camera()
			{
			}

			bool Camera::Init(Document::Node *node)
			{
				Document::Node *frustumNode = node->Resolve("frustum");
				if(frustumNode != nullptr) {
					frustum.Set(
						1.0f, 1.0f,
						frustumNode->children[1]->ValueAsFloat(),
						frustumNode->children[2]->ValueAsFloat(),
						frustumNode->children[0]->ValueAsFloat()
					);
				}
				return true;
			}
	
		} // namespace Components

	} // namespace Framework

} // namespace Maki