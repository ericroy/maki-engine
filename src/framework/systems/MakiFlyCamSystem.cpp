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

			FlyCamSystem::FlyCamSystem()
				: System(Component::TypeFlag_Camera|Component::TypeFlag_Transform)
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
					const Node &n = nodes[i];
					if(!n.camComp->active) {
						continue;
					}
										
					Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
					Vector4 perp(-1.0f, 0.0f, 0.0f, 0.0f);

					const Matrix44 &m = n.transComp->GetMatrix();
					dir = m * dir;
					perp = m * perp;
					Vector4 up = dir.Cross(perp);

					dir *= dt * c->GetFloat(InputState::Button_LeftThumbY) * 8.0f;
					perp *= dt * c->GetFloat(InputState::Button_LeftThumbX) * 8.0f;

					Vector4 pos = n.transComp->GetPosition();
					pos += dir + perp;
					pos += up * dt * c->GetFloat(InputState::Button_RightShoulder) * 5.0f;
					pos -= up * dt * c->GetFloat(InputState::Button_RightTrigger) * 5.0f;

					Vector3 angles;
					n.transComp->GetOrientation().ToEulerAngles(angles);			
			
					angles.z += dt * c->GetFloat(InputState::Button_RightThumbX) * 2.0f;
					angles.x += dt * c->GetFloat(InputState::Button_RightThumbY) * 2.0f;
					if(angles.x > MAKI_PI - 0.001f) {
						angles.x = MAKI_PI - 0.001f;
					} else if(angles.x < 0.001f) {
						angles.x = 0.001f;
					}
			
					n.transComp->SetMatrix(pos, Quaternion(angles));
				}
			}

			void FlyCamSystem::Add(Entity *e)
			{
				Node n;
				n.camComp = e->Get<Components::Camera>();
				n.transComp = e->Get<Components::Transform>();
				assert(n.camComp != nullptr && n.transComp != nullptr);
				nodes.push_back(n);
			}

			void FlyCamSystem::Remove(Entity *e)
			{
				Node n;
				n.camComp = e->Get<Components::Camera>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		} // namespace Systems

	} // namespace Framework

} // namespace Maki