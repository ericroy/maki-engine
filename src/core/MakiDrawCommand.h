#pragma once
#include "core/core_stdafx.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiMaterialManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiTextureSetManager.h"

namespace Maki
{
	namespace Core
	{
		namespace D3D
		{
			class RenderCoreImpl;
		}

	
		class Mesh;
		class Material;

		__declspec(align(16)) class DrawCommand : public Aligned<16>
		{
		friend class D3D::RenderCoreImpl;
		friend class DrawCommandList;

		public:
			static const int32 BITS_PER_MESH = 12;
			static const int32 BITS_PER_MATERIAL = 8;
			static const int32 BITS_PER_TEXTURE_SET = 7;
			static const int32 BITS_PER_SHADER_PROGRAM = 6;
			static const int32 BITS_PER_VERTEX_FORMAT = 4;

			enum TranslucencyType
			{
				TranslucencyType_Opaque = 0,
				TranslucencyType_Translucent,
			};

			// Listed from low to high bits (low to high sorting priority)
			struct KeyFields
			{
				// These fields contain the actual handle value to a resource.
				// Make sure there are enough bits to represent every handle up to the maximum size of the manager's pool.
				uint64 mesh : BITS_PER_MESH;
				uint64 meshManagerId : MeshManager::BITS_PER_MANAGER_ID;

				uint64 material : BITS_PER_MATERIAL;
				uint64 materialManagerId : MaterialManager::BITS_PER_MANAGER_ID;

				uint64 textureSet : BITS_PER_TEXTURE_SET;
				uint64 textureSetManagerId : TextureSetManager::BITS_PER_MANAGER_ID;

				uint64 shaderProgram : BITS_PER_SHADER_PROGRAM;
				uint64 shaderProgramManagerId : ShaderProgramManager::BITS_PER_MANAGER_ID;

				uint64 vertexFormat : BITS_PER_VERTEX_FORMAT;
				uint64 vertexFormatManagerId : VertexFormatManager::BITS_PER_MANAGER_ID;

				uint64 inverseDepth : 11;

				// Other higher priority sorting properties
				uint64 translucencyType : 1;
			};

		public:
			DrawCommand();
			DrawCommand(const MoveToken<DrawCommand> &other);
			~DrawCommand();

			inline uint64 GetKey() const { return key; }
			void SetMesh(Handle mesh);
			void SetMaterial(Handle material);
			inline void Clear();
			inline void Copy(const DrawCommand &other);

		private:
			DrawCommand(const DrawCommand &other) {}

		private:
			union
			{
				KeyFields fields;	
				uint64 key;
			};
		
			Handle mesh;
			Handle material;
			Handle textureSet;
			Handle shaderProgram;
			Handle vertexFormat;
		};

		static_assert(sizeof(DrawCommand::KeyFields) == 8, "DrawCommand key has exceeded 64 bits");



		inline void DrawCommand::Copy(const DrawCommand &other)
		{
			key = other.key;

			mesh = other.mesh;
			material = other.material;
			shaderProgram = other.shaderProgram;
			textureSet = other.textureSet;
			vertexFormat = other.vertexFormat;

			MeshManager::AddRef(mesh);
			MaterialManager::AddRef(material);
			ShaderProgramManager::AddRef(shaderProgram);
			TextureSetManager::AddRef(textureSet);
			VertexFormatManager::AddRef(vertexFormat);
		}
	
		inline void DrawCommand::Clear()
		{
			key = 0;

			MeshManager::Free(mesh);
			MaterialManager::Free(material);
			ShaderProgramManager::Free(shaderProgram);
			TextureSetManager::Free(textureSet);
			VertexFormatManager::Free(vertexFormat);
		}


	} // namespace Core

} // namespace Maki