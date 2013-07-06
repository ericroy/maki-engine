#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/MakiComponentPool.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			
			Light::Light()
				: Component(TYPE, DEPENDENCIES),
				fov(0.0f),
				lightType(RenderState::LightType_Directional),
				depthBuffer(HANDLE_NONE)
			{
				memset(&properties, 0, sizeof(properties));
				properties.diffuseColor = Vector4(1.0f);
				properties.specularColor = Vector4(1.0f);
				properties.attenuation = 1.0f;
				properties.spotFactor = 1.0f;

				SetLightType(RenderState::LightType_Directional);
				mapProperties.size = Vector2(-1.0f, -1.0f);
			}
			
			Light::~Light()
			{
				TextureManager::Free(depthBuffer);
			}
			
			bool Light::Init(Document::Node *props)
			{
				Document::Node *typeNode = props->Resolve("type.#0");
				if(typeNode != nullptr) {
					if(typeNode->ValueEquals("directional")) {
						SetLightType(RenderState::LightType_Directional);
					} else if(typeNode->ValueEquals("point")) {
						SetLightType(RenderState::LightType_Point);
					} else {
						Console::Error("Invalid light type: %s", typeNode->value);
						return false;
					}
				}

				SetOn(props->ResolveAsBool("on.#0", false));
				props->ResolveAsVectorN("diffuse", 3, properties.diffuseColor.vals);
				props->ResolveAsVectorN("ambient", 3, properties.ambientColor.vals);
				props->ResolveAsVectorN("specular", 3, properties.specularColor.vals);

				if(props->ResolveAsBool("shadows.#0", false)) {
					bool splitShadows = props->ResolveAsBool("split_shadows.#0", false);
					uint32 mapWidth = props->ResolveAsUInt("shadow_map_size.#0", 1024);
					uint32 mapHeight = props->ResolveAsUInt("shadow_map_size.#1", 1024);
					EnableShadows(mapWidth, mapHeight, splitShadows);
				}

				Document::Node *frustumNode = props->Resolve("frustum");
				if(frustumNode != nullptr) {
					fov = frustumNode->children[0]->ValueAsFloat();
					properties.widthHeightNearFar.x = frustumNode->children[1]->ValueAsFloat();
					properties.widthHeightNearFar.y = frustumNode->children[2]->ValueAsFloat();
					properties.widthHeightNearFar.z = frustumNode->children[3]->ValueAsFloat();
					properties.widthHeightNearFar.w = frustumNode->children[4]->ValueAsFloat();
				}

				return true;
			}

			Light *Light::Clone(bool prototype)
			{
				Light *c = ComponentPool<Light>::Get()->Create();
				c->fov = fov;
				c->lightType = lightType;
				c->properties = properties;
				c->mapProperties = mapProperties;

				TextureManager::AddRef(depthBuffer);
				c->depthBuffer = depthBuffer;
				return c;
			}

			void Light::EnableShadows(uint32 shadowMapWidth, uint32 shadowMapHeight, bool splitShadows)
			{
				properties.flags |= RenderState::LightFlag_Shadow;
				if(splitShadows) {
					properties.flags |= RenderState::LightFlag_SplitShadow;
				} else {
					properties.flags &= ~RenderState::LightFlag_SplitShadow;
				}

				if((uint32)mapProperties.size.x != shadowMapWidth || (uint32)mapProperties.size.y != shadowMapHeight) {
					TextureManager::Free(depthBuffer);
					mapProperties.size = Vector2((float)shadowMapWidth, (float)shadowMapHeight);
					depthBuffer = CoreManagers::Get()->textureManager->AllocTexture(Texture::TextureType_DepthStencil, (uint32)mapProperties.size.x, (uint32)mapProperties.size.y, 4);
				}
			}
			
			void Light::DisableShadows()
			{
				properties.flags &= ~RenderState::LightFlag_Shadow;
				TextureManager::Free(depthBuffer);
				mapProperties.size = Vector2(-1.0f, -1.0f);
			}


		} // namspace Components

	} // namespace Framework

} // namespace Maki
