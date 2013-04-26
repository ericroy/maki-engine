#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	class Light : public Entity
	{
	public:
		static std::function<Entity *()> CreateDebugWidget;

	public:
		Light(RenderState::LightType type, bool on = true, const Vector4 &diffuseColor = Vector4(1.0f));
		virtual ~Light();

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

		inline RenderState::LightType GetType() const { return type; }
		inline bool On() const { return (properties.flags & RenderState::LightFlag_On) != 0; }
		inline bool ShadowsEnabled() const { return (properties.flags & RenderState::LightFlag_Shadow) != 0; }
		inline bool SplitShadowsEnabled() const { return (properties.flags & RenderState::LightFlag_SplitShadow) != 0; }
		
	public:
		// Fov of zero means ortho projection
		float fov;
		RenderState::LightType type;
		RenderState::LightProperties properties;
		RenderState::ShadowMapProperties mapProperties;
		Handle depthBuffer;
	};

} // namespace Maki