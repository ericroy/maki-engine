#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			class Light : public Component
			{
			public:
				static const Component::Type TYPE = Type_Light;
				static std::function<Entity *()> CreateDebugWidget;

			public:
				Light();
				Light(RenderState::LightType lightType, bool on = true, const Vector4 &diffuseColor = Vector4(1.0f));
				virtual ~Light();

				virtual bool Init(Document::Node *node);
				virtual void Attach(Entity *owner);
				void EnableShadows(uint32 shadowMapWidth, uint32 shadowMapHeight, bool splitShadows = false);
				void DisableShadows();
		
				inline void SetOn(bool on)
				{
					if(on) {
						properties.flags |= RenderState::LightFlag_On;
					} else {
						properties.flags &= ~RenderState::LightFlag_On;
					}
				}

				inline RenderState::LightType GetLightType() const { return lightType; }
				inline bool On() const { return (properties.flags & RenderState::LightFlag_On) != 0; }
				inline bool ShadowsEnabled() const { return (properties.flags & RenderState::LightFlag_Shadow) != 0; }
				inline bool SplitShadowsEnabled() const { return (properties.flags & RenderState::LightFlag_SplitShadow) != 0; }
		
			public:
				// Fov of zero means ortho projection
				float fov;
				RenderState::LightType lightType;
				RenderState::LightProperties properties;
				RenderState::ShadowMapProperties mapProperties;
				Handle depthBuffer;
			};

		} // namespace Components

	} // namespace Framework

} // namespace Maki