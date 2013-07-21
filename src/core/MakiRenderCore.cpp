#include "core/core_stdafx.h"
#include "core/MakiRenderCore.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgram.h"

namespace Maki
{
	namespace Core
	{

		
		RenderCore::RenderCore()
			: Thread()
		{
		}

		RenderCore::~RenderCore()
		{
		}

		void RenderCore::Run()
		{
			Console::Info("Render core thread started");

			RenderPayload payload;
			while(true) {
				input.Get(payload);
				if(payload.cmd == RenderPayload::Command_Init) {
					Console::Info("Initializing render core");
					Init();
					Console::Info("Render core initialized");
					output.Put(payload);
				} else if(payload.cmd == RenderPayload::Command_Abort) {
					output.Put(payload);
					break;
				} else if(payload.cmd == RenderPayload::Command_Draw) {
#if MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
					payload.commands->Sort();
#endif
					Draw(*payload.state, *payload.commands);
					output.Put(payload);
				} else if(payload.cmd == RenderPayload::Command_Present) {
					Present();
					output.Put(payload);
				}
			}

			Console::Info("Render core thread stopped");
		}


		void RenderCore::SetPerFrameConstants(const Core::RenderState &state, const Core::Shader *s, char *buffer)
		{
			using namespace Core;

			int32 location = s->engineFrameUniformLocations[Shader::FrameUniform_View];
			if(location != -1) {
				memcpy(buffer + location, state.view.vals, 16*sizeof(float));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_Projection];
			if(location != -1) {
				memcpy(buffer + location, state.projection.vals, sizeof(state.projection));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraWithHeightNearFar];
			if(location != -1) {
				memcpy(buffer + location, &state.cameraWidthHeightNearFar, sizeof(state.cameraWidthHeightNearFar));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_CameraSplitDistances];
			if(location != -1) {
				memcpy(buffer + location, &state.cameraSplitDistances, sizeof(state.cameraSplitDistances));
			}


			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightViewProj];
			if(location != -1) {
				memcpy(buffer + location, state.lightViewProj, sizeof(state.lightViewProj));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightPositions];
			if(location != -1) {
				memcpy(buffer + location, state.lightPositions, sizeof(state.lightPositions));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightDirections];
			if(location != -1) {
				memcpy(buffer + location, state.lightDirections, sizeof(state.lightDirections));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightProperties];
			if(location != -1) {
				// Set all lighting slots here so that lights which are no longer in use get effectively turned off
				memcpy(buffer + location, state.lightProperties, sizeof(state.lightProperties));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_ShadowMapProperties];
			if(location != -1) {
				memcpy(buffer + location, state.shadowMapProperties, state.shadowLightCount*sizeof(RenderState::ShadowMapProperties));
			}
			location = s->engineFrameUniformLocations[Shader::FrameUniform_LightSplitRegions];
			if(location != -1) {
				memcpy(buffer + location, state.lightSplitRegions, state.cascadedShadowLightCount*RenderState::MAX_CASCADES*sizeof(RenderState::LightSplitRegion));
			}
		

			location = s->engineFrameUniformLocations[Shader::FrameUniform_GlobalAmbientColor];
			if(location != -1) {
				memcpy(buffer + location, &state.globalAmbientColor.x, sizeof(state.globalAmbientColor));
			}
		}


		void RenderCore::SetPerObjectConstants(const Core::Shader *s, char *buffer, const Core::Matrix44 &model, const Core::Matrix44 &modelView, const Core::Matrix44 &modelViewProjection)
		{
			using namespace Core;

#define MAKI_TRANSPOSE_MATRICES 0

			int32 location = s->engineObjectUniformLocations[Shader::ObjectUniform_Model];
			if(location != -1) {
#if MAKI_TRANSPOSE_MATRICES
				Matrix44 m;
				Matrix44::Transpose(model, m);
				memcpy(buffer + location, m.vals, sizeof(model));
#else
				memcpy(buffer + location, model.vals, sizeof(model));
#endif
			}

			location = s->engineObjectUniformLocations[Shader::ObjectUniform_ModelView];
			if(location != -1) {
#if MAKI_TRANSPOSE_MATRICES
				Matrix44 m;
				Matrix44::Transpose(modelView, m);
				memcpy(buffer + location, m.vals, sizeof(modelView));
#else
				memcpy(buffer + location, modelView.vals, sizeof(modelView));
#endif
			}

			location = s->engineObjectUniformLocations[Shader::ObjectUniform_ModelViewProjection];
			if(location != -1) {
#if MAKI_TRANSPOSE_MATRICES
				Matrix44 m;
				Matrix44::Transpose(modelViewProjection, m);
				memcpy(buffer + location, m.vals, sizeof(modelViewProjection));
#else
				memcpy(buffer + location, modelViewProjection.vals, sizeof(modelViewProjection));
#endif
			}
		}

		void RenderCore::BindMaterialConstants(const Core::Shader *s, bool isVertexShader, char *buffer, const Core::Material *mat)
		{
			using namespace Core;

			if(isVertexShader) {
				for(uint8 i = 0; i < mat->uniformCount; i++) {
					const Material::UniformValue &val = mat->uniformValues[i];
					if(val.vsLocation != -1) {
						memcpy(buffer + val.vsLocation, val.data, val.bytes);
					}
				}
			} else {
				for(uint8 i = 0; i < mat->uniformCount; i++) {
					const Material::UniformValue &val = mat->uniformValues[i];
					if(val.psLocation != -1) {
						memcpy(buffer + val.psLocation, val.data, val.bytes);
					}
				}
			}
		}


	} // namespace Core

} // namespace Maki