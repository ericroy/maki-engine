#pragma once
#include "ogl/ogl_stdafx.h"
#include "ogl/MakiOGLCommon.h"

namespace Maki
{
	namespace Core
	{
		class VertexFormat;

	} // namespace Core

	namespace OGL
	{

		struct Buffer
		{
			GLuint vbos[2];
			uint32 vertexCount;
			uint32 faceCount;
			uint32 indicesPerFace;
			uint32 bytesPerIndex;
			GLenum indexDataType;
			GLenum geometryType;

			void DeleteBuffers();
		};

		struct GPUVertexShader
		{
		public:
			GPUVertexShader();
			~GPUVertexShader();

			GLuint vs;
			GLuint uboPerFrame;
			GLuint uboPerObject;
			GLuint uboMaterial;
			char *scratchBuffer;
		};

		struct GPUPixelShader
		{
			GPUPixelShader();
			~GPUPixelShader();
			
			GLuint ps;
			GLuint uboPerFrame;
			GLuint uboPerObject;
			GLuint uboMaterial;
			char *scratchBuffer;
		};

		struct GPUTexture
		{
			GPUTexture();
			~GPUTexture();

			GLuint tex;
		};

	} // namespace OGL

} // namespace Maki
