#pragma once
#pragma comment(lib, "lua51.lib")
#pragma comment(lib, "SDL2.lib")

#if MAKI_USE_D3D
#	pragma comment (lib, "d3d11.lib")
#	pragma comment(lib, "dxgi.lib")
#else
#	pragma comment(lib, "opengl32.lib")
#	pragma comment(lib, "glu32.lib")
#endif

#if _DEBUG
#	pragma comment(lib, "ws2_32.lib")
#endif

#include "framework/MakiEntity.h"
#include "framework/MakiScene.h"
#include "framework/MakiScript.h"
#include "framework/MakiSceneLoader.h"
#include "framework/MakiCascadeHelper.h"
#include "framework/MakiDebugNetListener.h"
#include "framework/MakiDebugPhysicsRenderer.h"
#include "framework/MakiComponent.h"
#include "framework/MakiComponentPool.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiEntityPool.h"
#include "framework/components/MakiMeshComponent.h"
#include "framework/components/MakiNameComponent.h"
#include "framework/components/MakiPhysicsComponent.h"
#include "framework/components/MakiSceneNodeComponent.h"
#include "framework/components/MakiTransformComponent.h"
#include "framework/components/MakiSkeletonComponent.h"
#include "framework/components/MakiNavMeshComponent.h"
#include "framework/components/MakiLightComponent.h"
#include "framework/components/MakiCameraComponent.h"
#include "framework/components/MakiScriptComponent.h"
#include "framework/systems/MakiNameSystem.h"
#include "framework/systems/MakiRenderSystem.h"
#include "framework/systems/MakiPhysicsSystem.h"
#include "framework/systems/MakiSceneGraphSystem.h"
#include "framework/systems/MakiSkeletonSystem.h"
#include "framework/systems/MakiLightingSystem.h"
#include "framework/systems/MakiCameraSystem.h"
#include "framework/systems/MakiFlyCamSystem.h"
#include "framework/systems/MakiScriptingSystem.h"

#if MAKI_USE_D3D
#include "core/direct3d/MakiD3DRenderCore.h"
#else
#include "core/opengl/MakiOGLRenderCore.h"
#endif

namespace Maki {
	namespace Core {
#if MAKI_USE_D3D
		typedef D3D::D3DRenderCore RenderCoreImpl;
#else
		typedef OGL::OGLRenderCore RenderCoreImpl;
#endif
	}
}