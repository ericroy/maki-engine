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








	LightFactory::LightFactory()
		: EntityFactory(), on(true), diffuse(0.0f), ambient(0.0f), specular(0.0f), hasTarget(false), target(0.0f), shadows(false), splitShadows(false), hasFrustum(false)
	{
	}

	LightFactory::~LightFactory()
	{
	}

	bool LightFactory::PreCreate(Document::Node *node)
	{
		if(!EntityFactory::PreCreate(node)) {
			return false;
		}

		Document::Node *typeNode = node->Resolve("type.#0");
		if(typeNode != nullptr) {
			if(typeNode->ValueEquals("directional")) {
				type = RenderState::LightType_Directional;
			} else if(typeNode->ValueEquals("point")) {
				type = RenderState::LightType_Point;
			} else {
				Console::Error("Invalid light type: %s", typeNode->value);
				return false;
			}
		}

		on = node->ResolveAsBool("on.#0", true);
		node->ResolveAsVectorN("diffuse", 3, diffuse.vals);
		node->ResolveAsVectorN("ambient", 3, ambient.vals);
		node->ResolveAsVectorN("specular", 3, specular.vals);

		hasTarget = node->ResolveAsVectorN("target_pos", 3, target.vals);
		shadows = node->ResolveAsBool("shadows.#0", false);
		splitShadows = node->ResolveAsBool("split_shadows.#0", false);
		if(shadows) {
			mapWidth = node->ResolveAsUInt("shadow_map_size.#0", 1024);
			mapHeight = node->ResolveAsUInt("shadow_map_size.#1", 1024);
		}

		Document::Node *frustumNode = node->Resolve("frustum");
		if(frustumNode != nullptr) {
			hasFrustum = true;
			fov = frustumNode->children[0]->ValueAsFloat();
			widthHeightNearFar.x = frustumNode->children[1]->ValueAsFloat();
			widthHeightNearFar.y = frustumNode->children[2]->ValueAsFloat();
			widthHeightNearFar.z = frustumNode->children[3]->ValueAsFloat();
			widthHeightNearFar.w = frustumNode->children[4]->ValueAsFloat();
		}

		return true;
	}

	Light *LightFactory::Create()
	{
		return new Light(type, on, diffuse);
	}

	void LightFactory::PostCreate(Light *light)
	{
		EntityFactory::PostCreate(light);
		light->SetOn(on);
		light->properties.specularColor = specular;
		light->properties.ambientColor = specular;
			
		if(hasTarget) {
			Matrix44 lookAt;
			Matrix44::LookAt(pos, target, Vector4::UnitZ, lookAt);
			light->SetWorldMatrix(lookAt);
		}

		if(shadows) {
			light->EnableShadows(mapWidth, mapHeight, splitShadows);
		}

		if(hasFrustum) {
			light->properties.fov = fov;
			light->properties.widthHeightNearFar = widthHeightNearFar;
		}
	}


} // namespace Maki