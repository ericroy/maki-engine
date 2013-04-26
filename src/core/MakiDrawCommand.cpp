#include "core/core_stdafx.h"
#include "core/MakiDrawCommand.h"
#include "core/MakiEngine.h"
#include "core/MakiMeshManager.h"
#include "core/MakiVertexFormatManager.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"


namespace Maki
{

	DrawCommand::DrawCommand()
		: key(0),
		mesh(HANDLE_NONE),
		material(HANDLE_NONE),
		shaderProgram(HANDLE_NONE),
		textureSet(HANDLE_NONE),
		vertexFormat(HANDLE_NONE)
	{
	}

	DrawCommand::DrawCommand(DrawCommand &&other)
		: key(other.key),
		mesh(other.mesh),
		material(other.material),
		shaderProgram(other.shaderProgram),
		textureSet(other.textureSet),
		vertexFormat(other.vertexFormat)
	{
		MeshManager::AddRef(mesh);
		MaterialManager::AddRef(material);
		ShaderProgramManager::AddRef(shaderProgram);
		TextureSetManager::AddRef(textureSet);
		VertexFormatManager::AddRef(vertexFormat);
	}
	
	DrawCommand::~DrawCommand()
	{
		Clear();
	}

	
	void DrawCommand::SetMesh(Handle meshHandle)
	{
		MeshManager::Free(mesh);
		VertexFormatManager::Free(vertexFormat);
		
		if(meshHandle != HANDLE_NONE) {
			MeshManager::AddRef(meshHandle);
			mesh = meshHandle;
			fields.mesh = mesh & MeshManager::HANDLE_VALUE_MASK;
			fields.meshManagerId = mesh >> MeshManager::MANAGER_ID_SHIFT;
		
			const Mesh *m = MeshManager::Get(mesh);
			if(m->GetMeshFlag(Mesh::MeshFlag_HasTranslucency)) {
				fields.translucencyType = TranslucencyType_Translucent;
			}
			
			if(m->vertexFormat != HANDLE_NONE) {
				VertexFormatManager::AddRef(m->vertexFormat);
				vertexFormat = m->vertexFormat;
				fields.vertexFormat = vertexFormat & VertexFormatManager::HANDLE_VALUE_MASK;
				fields.vertexFormatManagerId = vertexFormat >> VertexFormatManager::MANAGER_ID_SHIFT;
			}
		}
	}

	void DrawCommand::SetMaterial(Handle materialHandle)
	{
		MaterialManager::Free(material);
		TextureSetManager::Free(textureSet);
		ShaderProgramManager::Free(shaderProgram);

		if(materialHandle != HANDLE_NONE) {
			MaterialManager::AddRef(materialHandle);
			material = materialHandle;
			fields.material = material & MaterialManager::HANDLE_VALUE_MASK;
			fields.materialManagerId = material >> MaterialManager::MANAGER_ID_SHIFT;

			const Material *mat = MaterialManager::Get(material);

			if(mat->textureSet != HANDLE_NONE) {
				TextureSetManager::AddRef(mat->textureSet);
				textureSet = mat->textureSet;
				fields.textureSet = textureSet & TextureSetManager::HANDLE_VALUE_MASK;
				fields.textureSetManagerId = textureSet >> TextureSetManager::MANAGER_ID_SHIFT;
			}

			if(mat->shaderProgram != HANDLE_NONE) {
				ShaderProgramManager::AddRef(mat->shaderProgram);
				shaderProgram = mat->shaderProgram;
				fields.shaderProgram = shaderProgram & ShaderProgramManager::HANDLE_VALUE_MASK;
				fields.shaderProgramManagerId = shaderProgram >> ShaderProgramManager::MANAGER_ID_SHIFT;
			}
		}
	}

} // namespace Maki