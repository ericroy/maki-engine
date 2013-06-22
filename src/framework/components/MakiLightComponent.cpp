#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiLightComponent.h"

namespace Maki
{
	std::function<Entity *()> LightComponent::CreateDebugWidget = nullptr;

	LightComponent::LightComponent()
	:	Component(COMPONENT_TYPE),
		lightType(RenderState::LightType_Directional),
		fov(0.0f),
		depthBuffer(HANDLE_NONE)
	{
		memset(&properties, 0, sizeof(properties));
		properties.diffuseColor = Vector4(1.0f);
		properties.specularColor = Vector4(1.0f);
		properties.attenuation = 1.0f;
		properties.spotFactor = 1.0f;

		mapProperties.size = Vector2(-1.0f, -1.0f);

		properties.flags |= (1<<lightType);
	}

	LightComponent::LightComponent(RenderState::LightType lightType, bool on, const Vector4 &diffuseColor)
	:	Component(COMPONENT_TYPE),
		lightType(lightType),
		fov(0.0f),
		depthBuffer(HANDLE_NONE)
	{
		memset(&properties, 0, sizeof(properties));
		properties.diffuseColor = diffuseColor;
		properties.specularColor = diffuseColor;
		properties.attenuation = 1.0f;
		properties.spotFactor = 1.0f;

		properties.flags |= (1<<lightType);
		if(on) {
			properties.flags |= RenderState::LightFlag_On;
		}

		mapProperties.size = Vector2(-1.0f, -1.0f);
	}

	LightComponent::~LightComponent()
	{
		TextureManager::Free(depthBuffer);
	}

	bool LightComponent::Init(Document::Node *node)
	{

		Document::Node *typeNode = node->Resolve("type.#0");
		if(typeNode != nullptr) {
			properties.flags &= ~(1<<lightType);
			
			if(typeNode->ValueEquals("directional")) {
				lightType = RenderState::LightType_Directional;
			} else if(typeNode->ValueEquals("point")) {
				lightType = RenderState::LightType_Point;
			} else {
				Console::Error("Invalid light type: %s", typeNode->value);
				return false;
			}

			properties.flags |= (1<<lightType);
		}

		SetOn(node->ResolveAsBool("on.#0", true));

		node->ResolveAsVectorN("diffuse", 3, properties.diffuseColor.vals);
		node->ResolveAsVectorN("ambient", 3, properties.ambientColor.vals);
		node->ResolveAsVectorN("specular", 3, properties.specularColor.vals);

		if(node->ResolveAsBool("shadows.#0", false)) {
			bool splitShadows = node->ResolveAsBool("split_shadows.#0", false);
			uint32 mapWidth = node->ResolveAsUInt("shadow_map_size.#0", 1024);
			uint32 mapHeight = node->ResolveAsUInt("shadow_map_size.#1", 1024);
			EnableShadows(mapWidth, mapHeight, splitShadows);
		}

		Document::Node *frustumNode = node->Resolve("frustum");
		if(frustumNode != nullptr) {
			properties.fov = frustumNode->children[0]->ValueAsFloat();
			properties.widthHeightNearFar.x = frustumNode->children[1]->ValueAsFloat();
			properties.widthHeightNearFar.y = frustumNode->children[2]->ValueAsFloat();
			properties.widthHeightNearFar.z = frustumNode->children[3]->ValueAsFloat();
			properties.widthHeightNearFar.w = frustumNode->children[4]->ValueAsFloat();
		}

		return true;
	}

	void LightComponent::Attach(Entity *owner)
	{
		Component::Attach(owner);

		owner->SetFlag(Entity::Flag_Draw, false);

#if _DEBUG
		if(CreateDebugWidget != nullptr) {
			Entity *debugWidget = CreateDebugWidget();
			if(debugWidget != nullptr) {
				owner->SendMessage(this, Message_DebugWidgetCreated, debugWidget, nullptr);
			}
		}
#endif
	}

	void LightComponent::DisableShadows()
	{
		properties.flags &= ~RenderState::LightFlag_Shadow;
		TextureManager::Free(depthBuffer);
		mapProperties.size = Vector2(-1.0f, -1.0f);
	}

	void LightComponent::EnableShadows(uint32 mapWidth, uint32 mapHeight, bool splitShadows)
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

} // namespace maki