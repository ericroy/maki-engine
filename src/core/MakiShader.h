#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiDocument.h"

namespace Maki
{
	namespace Core
	{

		class Shader
		{
		public:
			static const uint32 UNIFORM_NAME_MAX_LENGTH = 32;

			struct MaterialUniformLocation
			{
			public:
				MaterialUniformLocation();
				MaterialUniformLocation(int32 location, char *name);

			public:
				int32 location;
				char name[UNIFORM_NAME_MAX_LENGTH];
			};
					
			enum FrameUniform
			{
				FrameUniform_None = -1,
				FrameUniform_View = 0,
				FrameUniform_Projection,
				FrameUniform_ViewProjection,
				FrameUniform_LightPositions,
				FrameUniform_LightDirections,
				FrameUniform_LightViewProj,
				FrameUniform_LightProperties,
				FrameUniform_ShadowMapProperties,
				FrameUniform_LightSplitRegions,
				FrameUniform_CameraSplitDistances,
				FrameUniform_GlobalAmbientColor,
				FrameUniform_CameraWithHeightNearFar,
				FrameUniformCount
			};

			enum ObjectUniform
			{
				ObjectUniform_None = -1,
				ObjectUniform_Model = 0,
				ObjectUniform_ModelView,
				ObjectUniform_ModelViewProjection,
				ObjectUniformCount
			};
		
			static FrameUniform GetFrameUniformByName(const char *name);
			static ObjectUniform GetObjectUniformByName(const char *name);

			static const char *frameUniformNames[FrameUniformCount];
			static const char *objectUniformNames[ObjectUniformCount];

		public:
			Shader();
			~Shader();

			bool Init(Document::Node *shaderNode, const char *dataKey, const char *metaKey);
			int32 FindMaterialConstantLocation(const char *name);

		private:
			Shader(const Shader &other) {}

		public:
			int32 frameUniformBufferLocation;
			uint32 engineFrameUniformBytes;
			int32 engineFrameUniformLocations[FrameUniformCount];

			int32 objectUniformBufferLocation;
			uint32 engineObjectUniformBytes;
			int32 engineObjectUniformLocations[ObjectUniformCount];

			int32 materialUniformBufferLocation;
			uint32 materialUniformBytes;

			std::vector<MaterialUniformLocation> materialUniformLocations;

			
			char *programData;
			uint32 programDataBytes;

			intptr_t handle;
		};

		

	} // namespace Core

} // namespace Maki
