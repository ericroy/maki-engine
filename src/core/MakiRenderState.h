#pragma once
#include "core/core_stdafx.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiTextureManager.h"


namespace Maki
{
	namespace Core
	{

		class __declspec(align(SIMD_ALIGN)) RenderState : public Aligned<SIMD_ALIGN>
		{
		public:
			static const int32 MAX_LIGHTS = 8;
			static const int32 MAX_SHADOW_LIGHTS = 2;
			static const int32 MAX_SPLIT_SHADOW_LIGHTS = 2;
			static const int32 MAX_CASCADES = 5;

			enum RenderTarget
			{
				RenderTarget_Null = 0,
				RenderTarget_Default,
				RenderTarget_Custom
			};
			enum DepthStencil
			{
				DepthStencil_Null = 0,
				DepthStencil_Default,
				DepthStencil_Custom
			};
			enum CullMode
			{
				CullMode_None = 0,
				CullMode_Front,
				CullMode_Back
			};
			enum DepthTest
			{
				DepthTest_Disabled = 0,
				DepthTest_Less,
				DepthTest_Equal,
				DepthTest_LessEqual
			};
			enum LightType
			{
				LightType_Directional = 0,
				LightType_Point,
				LightType_Spot,
				LightTypeCount
			};

			enum LightFlag
			{
				LightFlag_TypeDirectional = 1<<LightType_Directional,
				LightFlag_TypePoint = 1<<LightType_Point,
				LightFlag_TypeSpot = 1<<LightType_Spot,

				LightFlag_On = 1<<3,
				LightFlag_Shadow = 1<<4,
				LightFlag_SplitShadow = 1<<5,
			};

			// Must match the light properties struct defined in shaders
			struct LightProperties
			{
				Vector4 ambientColor;
				Vector4 diffuseColor;
				Vector4 specularColor;
				Vector4 widthHeightNearFar;
				float attenuation;
				float spotFactor;
				float fov;
				uint32 flags;
			};

			struct ShadowMapProperties
			{
				Vector2 size;
				float pad[2];
			};

			struct CameraSplitDistances
			{
				float splits[MAX_CASCADES-1];
			};

			struct __declspec(align(SIMD_ALIGN)) LightSplitRegion : public Aligned<SIMD_ALIGN>
			{
				Matrix44 viewProj;
				Vector4 widthHeightNearFar;
			};

		public:
			RenderState()
			{
				memset(lightProperties, 0, sizeof(lightProperties));
				renderTargetType = RenderTarget_Default;
				depthStencilType = DepthStencil_Default;
				clearRenderTarget = false;
				clearDepthStencil = false;
				depthTest = DepthTest_Less;
				depthWrite = true;
				wireFrame = false;
				globalAmbientColor = Vector4(0.0f);
				cullMode = CullMode_Back;
				shaderVariant = ShaderProgram::Variant_Normal;
			
				lightCount = 0;
				shadowLightCount = 0;
				cascadedShadowLightCount = 0;

				renderTarget = HANDLE_NONE;
				depthStencil = HANDLE_NONE;
				for(uint32 i = 0; i < MAX_SHADOW_LIGHTS; i++) {
					shadowMaps[i] = HANDLE_NONE;
				}
			}

			~RenderState()
			{
				Clear();
			}

			void Copy(const RenderState &s)
			{
				*this = s;
				for(uint32 i = 0; i < MAX_SHADOW_LIGHTS; i++) {
					TextureManager::AddRef(shadowMaps[i]);
				}
				TextureManager::AddRef(renderTarget);
				TextureManager::AddRef(depthStencil);
			}

			void Clear()
			{
				for(uint32 i = 0; i < MAX_SHADOW_LIGHTS; i++) {
					TextureManager::Free(shadowMaps[i]);
				}
				TextureManager::Free(renderTarget);
				TextureManager::Free(depthStencil);
			}

		private:
			RenderState(const RenderState &other) {}

		public:
			// Camera data
			Matrix44 view;
			Matrix44 projection;
			CameraSplitDistances cameraSplitDistances;
			Vector4 cameraWidthHeightNearFar;

			// Light and shadow data
			uint32 lightCount;
			uint32 shadowLightCount;
			uint32 cascadedShadowLightCount;
			LightProperties lightProperties[MAX_LIGHTS];
			LightSplitRegion lightSplitRegions[MAX_SPLIT_SHADOW_LIGHTS][MAX_CASCADES];
			ShadowMapProperties shadowMapProperties[MAX_SHADOW_LIGHTS];
			Handle shadowMaps[MAX_SHADOW_LIGHTS];
			Matrix44 lightWorld[MAX_LIGHTS];
			Matrix44 lightView[MAX_LIGHTS];
			Matrix44 lightProj[MAX_LIGHTS];
			Matrix44 lightViewProj[MAX_LIGHTS];
			Vector4 lightPositions[MAX_LIGHTS];
			Vector4 lightDirections[MAX_LIGHTS];

			// Basic render state
			uint32 windowWidth;
			uint32 windowHeight;
			Rect viewPortRect;
			RenderState::RenderTarget renderTargetType;
			Handle renderTarget;
			RenderState::DepthStencil depthStencilType;
			Handle depthStencil;
			DepthTest depthTest;
			bool depthWrite;
			bool wireFrame;
			CullMode cullMode;
			Vector4 globalAmbientColor;
			ShaderProgram::Variant shaderVariant;

			// Actions
			bool clearRenderTarget;
			Vector4 renderTargetClearValue;
			bool clearDepthStencil;
			float depthClearValue;		
		};


	} // namespace Core

} // namespace Maki