#pragma once
#include "oglrendercore/oglrendercore_stdafx.h"
#include "oglrendercore/MakiOGLCommon.h"


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

		namespace OGL
		{
	
			class MAKI_OGLRENDERCORE_API OGLRenderCore : public RenderCore
			{
			private:
				static const int32 SHADOW_MAP_SLOT_INDEX_START = 8;

			public:
				OGLRenderCore(Window *window, const Config *config);
				virtual ~OGLRenderCore();

				void *UploadBuffer(void *buffer, VertexFormat *vf, char *vertexData, uint32 vertexCount, char *indexData, uint32 faceCount, uint8 indicesPerFace, uint8 bytesPerIndex, bool dynamic);
				void FreeBuffer(void *buffer);
				bool CreateShaderProgram(ShaderProgram *s);
				void DeleteShaderProgram(ShaderProgram *s);
				bool CreateTexture(Texture *t, char *data, uint32 dataLength);
				bool CreateEmptyTexture(Texture *t, uint8 channels);
				bool CreateRenderTarget(Texture *t);
				bool CreateDepthTexture(Texture *t);
				void WriteToTexture(Texture *t, int32 dstX, int32 dstY, int32 srcX, int32 srcY, uint32 srcWidth, uint32 srcHeight, uint32 srcPitch, uint8 channels, char *srcData);
				void DeleteTexture(Texture *t);

			protected:
				void Init();
				void Draw(const RenderState &state, const DrawCommandList &commands);
				void Present();

			private:
				void Resized(uint32 newWidth, uint32 newHeight);
				void SetDepthState(RenderState::DepthTest test, bool write);
				void SetRasterizerState(RenderState::CullMode cullMode, bool wireFrame);
				void SetRenderTargetAndDepthStencil(RenderState::RenderTarget renderTargetType, Handle renderTarget, RenderState::DepthStencil depthStencilType, Handle depthStencil);
				//void SetPerFrameConstants(const RenderState &state, const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped);
				//void SetPerObjectConstants(const Shader *s, D3D11_MAPPED_SUBRESOURCE &mapped, const Matrix44 &model, const Matrix44 &modelView, const Matrix44 &modelViewProjection);
				//void BindMaterialConstants(const Shader *s, bool isVertexShader, D3D11_MAPPED_SUBRESOURCE &mapped, const Material *mat);
				void BindTextures(const ShaderProgram *shader, const TextureSet *ts);
				bool CreatePixelShader(Shader *ps);
				bool CreateVertexShader(Shader *vs);

			private:
				Window *window;
				SDL_GLContext context;
				uint32 windowWidth;
				uint32 windowHeight;


				bool vsync;
				uint32 maxVertexFormatsPerVertexShader;

				void *nullArray[SHADOW_MAP_SLOT_INDEX_START+RenderState::MAX_LIGHTS];
			};

		} // namespace OGL
	
	} // namespace Core

} // namespace Maki
