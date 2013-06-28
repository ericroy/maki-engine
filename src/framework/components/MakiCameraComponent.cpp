#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiCameraComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Camera::Camera()
				: Component(TYPE, DEPENDENCIES),
				active(true),
				frustum(0.0f, 0.0f, 0.0f, 1.0f, 50.0f)
			{

			}

			Camera::~Camera()
			{
			}

			bool Camera::Init(Document::Node *props)
			{
				active = props->ResolveAsBool("active.#0", true);

				Document::Node *frustumNode = props->Resolve("frustum");
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


		} // namspace Components

	} // namespace Framework

} // namespace Maki