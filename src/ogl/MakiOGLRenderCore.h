#pragma once
#include "ogl/ogl_stdafx.h"

#pragma comment(lib, "MakiCore.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

namespace Maki
{
	namespace Core
	{
		
		class Window;
		class InputLayoutCache;
		class VertexFormat;
		class ShaderProgram;
		class TextureSet;
		class TextureManager;
		class Material;
		class Texture;
		class Config;

	} // namespace Core

	namespace OGL
	{


		class OGLRenderCore : public Core::RenderCore
		{
		private:
			static const int32 SHADOW_MAP_SLOT_INDEX_START = 8;

		public:
			OGLRenderCore(Core::Window *window, const Core::Config *config);
			virtual ~OGLRenderCore();

			void *UploadBuffer(void *buffer, Core::VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic);
			void FreeBuffer(void *buffer);
			bool CreateShaderProgram(Core::ShaderProgram *s);
			void DeleteShaderProgram(Core::ShaderProgram *s);
			bool CreateTexture(Core::Texture *t, char *data, uint32 dataLength);
			bool CreateEmptyTexture(Core::Texture *t, uint8 channels);
			bool CreateRenderTarget(Core::Texture *t);
			bool CreateDepthTexture(Core::Texture *t);
			void WriteToTexture(Core::Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
			void DeleteTexture(Core::Texture *t);

		protected:
			void Init();
			void Draw(const Core::RenderState &state, const Core::DrawCommandList &commands);
			void Present();

		private:
			void Resized(uint32 newWidth, uint32 newHeight);
			void SetDepthState(Core::RenderState::DepthTest test, bool write);
			void SetRasterizerState(Core::RenderState::CullMode cullMode, bool wireFrame);
			void SetRenderTargetAndDepthStencil(Core::RenderState::RenderTarget renderTargetType, Handle renderTarget, Core::RenderState::DepthStencil depthStencilType, Handle depthStencil);
			//void SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped);
			//void SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection);
			//void BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat);
			void BindTextures(const Core::ShaderProgram *shader, const Core::TextureSet *ts);
			bool CreatePixelShader(Core::Shader *ps);
			bool CreateVertexShader(Core::Shader *vs);

			void AcquireContextInRenderThread();
			void AcquireContextInMainThread();

		private:
			Core::Window *window;
			SDL_GLContext context;
			uint32 windowWidth;
			uint32 windowHeight;

			GLuint frameBuffer;

			GLuint defaultRenderTarget;
			GLuint defaultDepthStencil;

			GLuint currentRenderTarget;
			GLuint currentDepthStencil;

			bool vsync;
			bool contextCurrentInRenderThread;
		};


	} // namespace OGL

} // namespace Maki
