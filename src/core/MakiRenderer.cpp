#include "core/core_stdafx.h"
#include "core/MakiRenderer.h"
#include "core/MakiConfig.h"
#include "core/MakiRenderPayload.h"
#include "core/MakiRenderCore.h"

namespace Maki
{

	Renderer::Renderer(Window *window, RenderCore *core, const Config *config)
	:	window(window),
		core(core),
		lightDirtyFlags(0),
		state(nullptr),
		commands(nullptr)
	{
		uint32 maxDrawCommands = config->GetUint("engine.max_draw_commands_per_pass", DEFAULT_MAX_DRAW_COMMANDS_PER_PASS);
		for(uint32 i = 0; i < MAX_RENDER_PAYLOADS; i++) {
			renderStates.push_back(new RenderState());
			commandLists.push_back(new DrawCommandList(maxDrawCommands));
		}

		current.windowWidth = window->width;
		current.windowHeight = window->height;
		PrepareNextRenderState();

		// Kickoff the core thread, and make sure that it is initialized before we continue
		core->Start();
		RenderPayload payload(RenderPayload::Command_Init);
		core->input.Put(payload);
		core->output.Get(payload);
		assert(payload.cmd == RenderPayload::Command_Init);

		if(config->GetBool("engine.wire_frame", false)) {
			SetWireFrameEnabled(true);
		}
	}
	
	Renderer::~Renderer()
	{
		if(core != nullptr) {
			core->input.Put(RenderPayload(RenderPayload::Command_Abort));
			core->Join();
			SyncWithCore();
		}

		for(uint32 i = 0; i < renderStates.size(); i++) {
			SAFE_DELETE(renderStates[i]);
			SAFE_DELETE(commandLists[i]);
		}

		state->Clear();
		commands->Clear();

		SAFE_DELETE(state);
		SAFE_DELETE(commands);
	}

	bool Renderer::Begin()
	{
		if(core != nullptr) {
			SyncWithCore();
			current.windowWidth = window->width;
			current.windowHeight = window->height;
			current.clearRenderTarget = false;
			current.clearDepthStencil = false;
			return true;
		}
		return false;
	}

	void Renderer::End()
	{
		RenderPayload payload(RenderPayload::Command_Present);
		core->input.Put(payload);
	}

	void Renderer::SyncWithCore()
	{
		uint32 outstanding = MAX_RENDER_PAYLOADS - renderStates.size() - 1;
		while(outstanding != 0) {
			RenderPayload temp;
			core->output.Get(temp);
			if(temp.state != nullptr) {
				temp.state->Clear();
				temp.commands->Clear();
				renderStates.push_back(temp.state);
				commandLists.push_back(temp.commands);
				outstanding--;
			}
		}
	}

	void Renderer::PrepareNextRenderState()
	{
		assert(renderStates.size() > 0);

		state = renderStates.back();
		renderStates.pop_back();
		commands = commandLists.back();
		commandLists.pop_back();
	}

	void Renderer::SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil)
	{
		current.renderTargetType = renderTargetType;
		if(renderTargetType == RenderState::RenderTarget_Custom) {
			TextureManager::AddRef(renderTarget);
			TextureManager::Free(current.renderTarget);
			current.renderTarget = renderTarget;
		} else {
			TextureManager::Free(current.renderTarget);
		}

		current.depthStencilType = depthStencilType;
		if(depthStencilType == RenderState::DepthStencil_Custom) {
			TextureManager::AddRef(depthStencil);
			TextureManager::Free(current.depthStencil);
			current.depthStencil = depthStencil;
		} else {
			TextureManager::Free(current.depthStencil);
		}
	}

	void Renderer::SetOrthoProjection(const Frustum &frustum)
	{
		current.cameraWidthHeightNearFar = Vector4(frustum.GetWidth(), frustum.GetHeight(), frustum.nearPlane, frustum.farPlane);
		Matrix44::Ortho(frustum, current.projection);
	}

	void Renderer::SetPerspectiveProjection(const Frustum &frustum)
	{
		current.cameraWidthHeightNearFar = Vector4(frustum.GetWidth(), frustum.GetHeight(), frustum.nearPlane, frustum.farPlane);
		Matrix44::Perspective(frustum, current.projection);
	}

	void Renderer::SetLight(uint32 lightIndex, const RenderState::LightProperties *props, const RenderState::ShadowMapProperties *shadProps, const Matrix44 *matrix, float fov, Handle depthBuffer)
	{
		auto res = ResourceProvider::Get();

		lightDirtyFlags |= (1<<lightIndex);

		if(props == nullptr || (props->flags & RenderState::LightFlag_On) == 0) {
			current.lightProperties[lightIndex].flags &= ~RenderState::LightFlag_On;
			memset(&current.lightProperties[lightIndex], 0, sizeof(RenderState::LightProperties));
			TextureManager::Free(current.shadowMaps[lightIndex]);
			return;
		}

		memcpy(&current.lightProperties[lightIndex], props, sizeof(RenderState::LightProperties));
		
		assert(matrix != nullptr);
		current.lightWorld[lightIndex] = *matrix;
		Matrix44::AffineInverse(*matrix, current.lightView[lightIndex]);

		if(fov == 0.0f) {
			float wo2 = props->widthHeightNearFar.x / 2.0f;
			float ho2 = props->widthHeightNearFar.y / 2.0f;
			Matrix44::Ortho(-wo2, wo2, -ho2, ho2, props->widthHeightNearFar.z, props->widthHeightNearFar.w, current.lightProj[lightIndex]);
		} else {
			Matrix44::Perspective(fov, props->widthHeightNearFar.x / props->widthHeightNearFar.y, props->widthHeightNearFar.z, props->widthHeightNearFar.w, current.lightProj[lightIndex]);
		}
		
		if((props->flags & RenderState::LightFlag_Shadow) != 0 && depthBuffer != HANDLE_NONE) {
			assert(lightIndex < RenderState::MAX_SHADOW_LIGHTS);
			TextureManager::AddRef(depthBuffer);
			TextureManager::Free(current.shadowMaps[lightIndex]);
			current.shadowMaps[lightIndex] = depthBuffer;
			current.shadowMapProperties[lightIndex] = *shadProps;
		} else {
			if(lightIndex < RenderState::MAX_SHADOW_LIGHTS) {
				current.shadowMapProperties[lightIndex].size = Vector2(0.0f);
				TextureManager::Free(current.shadowMaps[lightIndex]);
			}
		}
	}

	void Renderer::SetLightCascade(uint32 lightIndex, uint32 cascadeIndex, const Frustum &frustum)
	{
		assert(lightIndex < current.cascadedShadowLightCount);
		assert(cascadeIndex < RenderState::MAX_CASCADES);

		RenderState::LightSplitRegion &region = current.lightSplitRegions[lightIndex][cascadeIndex];

		Matrix44::Ortho(frustum, region.viewProj);

		// Light must have been set already!!!!!!
		region.viewProj = region.viewProj * current.lightView[lightIndex];
		
		region.widthHeightNearFar = Vector4(frustum.GetWidth(), frustum.GetHeight(), frustum.nearPlane, frustum.farPlane);
	}

	void Renderer::Submit()
	{

		// Finish preparing lighting info now that view matrix is certain
		//if(lightDirtyFlags != 0) {
			const Vector4 pos(0.0f);
			const Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
			for(uint32 i = 0; i < current.lightCount; i++) {
				//if((lightDirtyFlags & (1<<i)) != 0 && (current.lightProperties[i].flags & RenderState::LightFlag_On) != 0) {
					Matrix44 toViewSpace = current.view * current.lightWorld[i];
					current.lightPositions[i] = toViewSpace * pos;
					current.lightDirections[i] = toViewSpace * dir;
					current.lightViewProj[i] = current.lightProj[i] * current.lightView[i];
				//}
			}
			lightDirtyFlags = 0;
		//}

		// Duplicate our current renderstate (to be sent with the payload)
		state->Copy(current);

#if !MAKI_SORT_DRAW_COMMANDS_IN_RENDER_THREAD
		commands->Sort();
#endif

		// Send renderstate and commands to the core for processing
		RenderPayload payload(RenderPayload::Command_Draw);
		payload.state = state;
		payload.commands = commands;
		core->input.Put(payload);

		state = nullptr;
		commands = nullptr;

		current.clearRenderTarget = false;
		current.clearDepthStencil = false;

		PrepareNextRenderState();
	}

} // namespace Maki