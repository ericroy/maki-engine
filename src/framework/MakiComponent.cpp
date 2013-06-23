#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/components/MakiTransformComponent.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiCameraComponent.h"
#include "framework/components/MakiBillboardComponent.h"
#include "framework/components/MakiCharacterComponent.h"
#include "framework/components/MakiSceneNodeComponent.h"

namespace Maki
{

	Component *Component::Create(const char *type)
	{
		auto pool = ComponentPool::Get();
		assert(pool);

		if(strcmp(type, "transform") == 0) {
			return pool->Create<TransformComponent>();
		} else if(strcmp(type, "scene_node") == 0) {
			return pool->Create<SceneNodeComponent>();
		} else if(strcmp(type, "mesh") == 0) {
			return pool->Create<MeshComponent>();
		} else if(strcmp(type, "light") == 0) {
			return pool->Create<LightComponent>();
		} else if(strcmp(type, "camera") == 0) {
			return pool->Create<CameraComponent>();
		} else if(strcmp(type, "billboard") == 0) {
			return pool->Create<BillboardComponent>();
		} else if(strcmp(type, "character") == 0) {
			return pool->Create<CharacterComponent>();
		}
		return nullptr;
	}
	

} // namespace Maki