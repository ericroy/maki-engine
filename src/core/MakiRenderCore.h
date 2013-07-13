#pragma once
#include "core/core_stdafx.h"
#include <mutex>
#include "core/MakiThread.h"
#include "core/MakiSafeQueue.h"
#include "core/MakiRenderPayload.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiRenderState.h"

namespace Maki
{
	namespace Core
	{
		class VertexFormat;

		
		

		class RenderCore : public Thread
		{
		public:
			enum Type
			{
				Type_D3D = 0,
				Type_OGL,
			};

		public:
			RenderCore();
			virtual ~RenderCore();
			void Run();
	
			// GPU resource creation, updates, destruction
			// These all acquire the execution mutex for the render core
			virtual void *UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic) = 0;
			virtual void FreeBuffer(void *buffer) = 0;
			virtual bool CreateShaderProgram(ShaderProgram *s) = 0;
			virtual void DeleteShaderProgram(ShaderProgram *s) = 0;
			virtual bool CreateTexture(Texture *t, char *data, uint32 dataLength) = 0;
			virtual bool CreateEmptyTexture(Texture *t, uint8 channels) = 0;
			virtual bool CreateRenderTarget(Texture *t) = 0;
			virtual bool CreateDepthTexture(Texture *t) = 0;
			virtual void WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData) = 0;
			virtual void DeleteTexture(Texture *t) = 0;

		protected:
			virtual void Init() = 0;
			virtual void Draw(const RenderState &state, const DrawCommandList &commands) = 0;
			virtual void Present() = 0;

		public:
			SafeQueue<RenderPayload> input;
			SafeQueue<RenderPayload> output;

		protected:
			std::mutex mutex;
		};

	} // namespace Core

} // namespace Maki