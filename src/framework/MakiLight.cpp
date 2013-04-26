#include "framework/framework_stdafx.h"
#include "framework/MakiLight.h"

namespace Maki
{
	std::function<Entity *()> Light::CreateDebugWidget = nullptr;


	Light::Light(RenderState::LightType type, bool on, const Vector4 &diffuseColor)
	:	Entity(Flag_Update|Flag_ProcessChildren),
		type(type),
		fov(0.0f),
		depthBuffer(HANDLE_NONE)
	{
		memset(&properties, 0, sizeof(properties));
		properties.diffuseColor = diffuseColor;
		properties.specularColor = diffuseColor;
		properties.attenuation = 1.0f;
		properties.spotFactor = 1.0f;

		properties.flags |= (1<<type);
		if(on) {
			properties.flags |= RenderState::LightFlag_On;
		}

		mapProperties.size = Vector2(-1.0f, -1.0f);
		
#if _DEBUG
		if(CreateDebugWidget != nullptr) {
			Entity *debugWidget = CreateDebugWidget();
			if(debugWidget != nullptr) {
				AddChild(debugWidget);
			}
		}
#endif
	}

	Light::~Light()
	{
		TextureManager::Free(depthBuffer);
	}

	void Light::DisableShadows()
	{
		properties.flags &= ~RenderState::LightFlag_Shadow;
		TextureManager::Free(depthBuffer);
		mapProperties.size = Vector2(-1.0f, -1.0f);
	}

	void Light::EnableShadows(uint32 mapWidth, uint32 mapHeight, bool splitShadows)
	{
		properties.flags |= RenderState::LightFlag_Shadow;
		if(splitShadows) {
			properties.flags |= RenderState::LightFlag_SplitShadow;
		} else {
			properties.flags &= ~RenderState::LightFlag_SplitShadow;
		}

		if((uint32)mapProperties.size.x != mapWidth || (uint32)mapProperties.size.y != mapHeight) {
			TextureManager::Free(depthBuffer);
			mapProperties.size = Vector2((float)mapWidth, (float)mapHeight);
			depthBuffer = ResourceProvider::Get()->textureManager->AllocTexture(Texture::TextureType_DepthStencil, (uint32)mapProperties.size.x, (uint32)mapProperties.size.y, 4);
		}
	}


} // namespace Maki