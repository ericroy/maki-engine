#pragma once
#pragma comment(lib, "xinput.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define NOMINMAX

#include "core/MakiUtil.h"
#include "core/MakiAllocator.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiAssetManifest.h"
#include "core/MakiArchive.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiDocument.h"
#include "core/MakiSkeleton.h"
#include "core/MakiConfig.h"
#include "core/MakiTimer.h"
#include "core/MakiResourceProvider.h"
#include "core/MakiTextureManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiFontManager.h"
#include "core/MakiSkeletonManager.h"
#include "core/MakiSkeletonAnimationManager.h"
#include "core/windows/MakiWin32Window.h"
#include "core/direct3d/MakiDirect3DRenderCore.h"
