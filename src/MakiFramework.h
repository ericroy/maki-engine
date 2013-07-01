#pragma once
#pragma comment(lib, "lua51.lib")

#if _DEBUG
#pragma comment(lib, "ws2_32.lib")
#endif


#include "framework/MakiEntity.h"
#include "framework/MakiScene.h"
#include "framework/MakiScript.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiCascadeHelper.h"
#include "framework/MakiDebugNetListener.h"
#include "framework/MakiDebugPhysicsRenderer.h"
#include "framework/MakiPhysicsWorld.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiCameraComponent.h"
#include "framework/components/MakiScriptComponent.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/systems/MakiSceneGraphSystem.h"
#include "framework/systems/MakiSkeletonSystem.h"
#include "framework/systems/MakiLightingSystem.h"
#include "framework/systems/MakiCameraSystem.h"
#include "framework/systems/MakiFlyCamSystem.h"
#include "framework/systems/MakiScriptingSystem.h"
