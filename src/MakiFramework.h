#pragma once

#if _DEBUG
#pragma comment(lib, "ws2_32.lib")
#endif

#include "framework/MakiEntity.h"
#include "framework/MakiScene.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiCamera.h"
#include "framework/MakiLight.h"
#include "framework/MakiMeshEntity.h"
#include "framework/MakiSkinnedMeshEntity.h"
#include "framework/MakiScreenString.h"
#include "framework/MakiCascadeHelper.h"
#include "framework/MakiDebugTextureWidget.h"
#include "framework/MakiDebugNetListener.h"
#include "framework/MakiDebugPhysicsRenderer.h"
#include "framework/MakiPhysicsLink.h"
#include "framework/MakiPhysicsWorld.h"