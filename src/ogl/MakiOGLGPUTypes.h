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

		struct GPUShader
		{
		public:
			GPUShader();
			~GPUShader();

			GLuint sh;
			GLuint uboPerFrame;
			GLuint uboPerObject;
			GLuint uboMaterial;
			char *scratchBuffer;

			GLint textureSamplerLocations[SHADOW_MAP_SLOT_INDEX_START+Core::RenderState::MAX_LIGHTS];
		};

		struct GPUTexture
		{
			GPUTexture();
			~GPUTexture();

			GLuint tex;
		};

	} // namespace OGL

} // namespace Maki
