#pragma once
#include "framework/framework_stdafx.h"
#include "framework/systems/MakiFlyCamSystem.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Systems
		{

			FlyCamSystem::FlyCamSystem(uint32 messageQueueSize)
				: System2(Component::TypeFlag_Camera|Component::TypeFlag_Transform, 0, messageQueueSize, "FlyCamSystem")
			{
			}

			FlyCamSystem::~FlyCamSystem()
			{
			}

			void FlyCamSystem::Update(float dt)
			{
				Engine *eng = Engine::Get();
				InputState::Controller *c = eng->inputState->GetPlayer(0)->controller;
				if(c == nullptr) {
					return;
				}

				const uint32 nodeCount = nodes.size();
				for(uint32 i = 0; i < nodeCount; i++) {
					if(!nodes[i].Get<Components::Camera>()->active) {
						continue;
					}
					Components::Transform *transComp = nodes[i].Get<Components::Transform>();
										
					Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
					Vector4 perp(-1.0f, 0.0f, 0.0f, 0.0f);

					const Matrix44 &m = transComp->GetMatrix();
					dir = m * dir;
					perp = m * perp;
					Vector4 up = dir.Cross(perp);

					dir *= dt * -c->GetFloat(InputState::Button_LeftThumbY) * 8.0f;
					perp *= dt * -c->GetFloat(InputState::Button_LeftThumbX) * 8.0f;

					Vector4 pos = transComp->GetPosition();
					pos += dir + perp;
					pos += up * dt * c->GetFloat(InputState::Button_RightShoulder) * 5.0f;
					pos -= up * dt * c->GetFloat(InputState::Button_RightTrigger) * 5.0f;

					Vector3 angles;
					transComp->GetOrientation().ToEulerAngles(angles);			
			
					angles.y += dt * -c->GetFloat(InputState::Button_RightThumbX) * 2.0f;
					angles.x += dt * -c->GetFloat(InputState::Button_RightThumbY) * 2.0f;
					if(angles.x > MAKI_PI/2.0f - 0.001f) {
						angles.x = MAKI_PI/2.0f - 0.001f;
					} else if(angles.x < -MAKI_PI/2.0f + 0.001f) {
						angles.x = -MAKI_PI/2.0f + 0.001f;
					}
			
					transComp->SetMatrix(pos, Quaternion(angles));
				}
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki