#pragma once

#if _DEBUG
#pragma comment(lib, "ws2_32.lib")
#endif


#include "framework/MakiEntity.h"
#include "framework/MakiScene.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiScreenString.h"
#include "framework/MakiCascadeHelper.h"
#include "framework/MakiDebugTextureWidget.h"
#include "framework/MakiDebugNetListener.h"
#include "framework/MakiDebugPhysicsRenderer.h"
#include "framework/MakiDebugArmature.h"
#include "framework/MakiPhysicsLink.h"
#include "framework/MakiPhysicsWorld.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/components/MakiCameraComponent.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/systems/MakiSceneGraphSystem.h"
#include "framework/systems/MakiSkeletonSystem.h"
