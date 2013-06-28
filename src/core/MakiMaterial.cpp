#include "core/core_stdafx.h"
#include "core/MakiMaterial.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureSet.h"
#include "core/MakiDocument.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace Maki
{
	namespace Core
	{

		Material::Material()
			: Resource(),
			uniformCount(0),
			textureSet(HANDLE_NONE),
			shaderProgram(HANDLE_NONE)
		{
		}

		Material::Material(const MoveToken<Material> &other)
			: Resource((const MoveToken<Resource> &)other),
			uniformCount(other.obj->uniformCount),
			textureSet(other.obj->textureSet),
			shaderProgram(other.obj->shaderProgram)
		{
			other.obj->uniformCount = 0;
			other.obj->textureSet = HANDLE_NONE;
			other.obj->shaderProgram = HANDLE_NONE;
			memcpy(uniformValues, other.obj->uniformValues, sizeof(UniformValue)*uniformCount);
			memset(other.obj->uniformValues, 0, sizeof(UniformValue)*MAX_UNIFORMS);
		}

		Material::Material(const Material &other)
		:	Resource(other),
			uniformCount(other.uniformCount),
			textureSet(other.textureSet),
			shaderProgram(other.shaderProgram)
		{
			TextureSetManager::AddRef(textureSet);
			ShaderProgramManager::AddRef(shaderProgram);
		
			memcpy(uniformValues, other.uniformValues, sizeof(UniformValue)*uniformCount);
			for(uint32 i = 0; i < uniformCount; i++) {
				UniformValue &val = uniformValues[i];
				char *oldData = val.data;
				val.data = (char *)Allocator::Malloc(val.bytes);
				memcpy(val.data, oldData, val.bytes);
			}
		}

		Material::~Material()
		{
			ShaderProgramManager::Free(shaderProgram);
			TextureSetManager::Free(textureSet);
		}

		void Material::SetShaderProgram(Rid shaderRid)
		{
			Handle newShaderProgram = CoreManagers::Get()->shaderProgramManager->Load(shaderRid);
			ShaderProgramManager::Free(shaderProgram);
			shaderProgram = newShaderProgram;
		}
	
		void Material::SetTextures(uint8 count, Rid *textureRids)
		{
			Handle newTextureSet = CoreManagers::Get()->textureSetManager->Load(count, textureRids);
			TextureSetManager::Free(textureSet);
			textureSet = newTextureSet;
		}

		int32 Material::PushConstant(const char *key, uint32 bytes, char *data)
		{
			if(shaderProgram == HANDLE_NONE) {
				Console::Warning("Cannot push constant, must set shader program first");
				return -1;
			}
			if(uniformCount >= MAX_UNIFORMS) {
				Console::Error("Cannot push constant, limit of %d constants reached", MAX_UNIFORMS);
				return -1;
			}

			ShaderProgram *shader = ShaderProgramManager::Get(shaderProgram);

			// Find constant in vertex shader
			int32 vsLocation = shader->vertexShader.FindMaterialConstantLocation(key);
			int32 psLocation = shader->pixelShader.FindMaterialConstantLocation(key);

			if(vsLocation == -1 && psLocation == -1) {
				Console::Error("Constant not found in vertex or pixel shader: %s", key);
				return -1;
			}

			int32 index = uniformCount++;
			UniformValue &value = uniformValues[index];
			value.psLocation = psLocation;
			value.vsLocation = vsLocation;
			value.bytes = bytes;
			value.data = data;
			return index;
		}

		bool Material::Load(Rid rid)
		{
			CoreManagers *res = CoreManagers::Get();
			Engine *eng = Engine::Get();

			if(rid == RID_NONE) {
				Console::Error("Failed to load material, rid is RID_NONE");
				return false;
			}

			Document doc;
			if(!doc.Load(rid)) {
				Console::Error("Failed to parse material file <rid %d>", rid);
				return false;
			}

			ShaderProgram *shader = nullptr;
			Rid shaderProgramRid = RID_NONE;

			char *shaderProgramPath = doc.root->ResolveValue("shader.#0");
			if(shaderProgramPath == nullptr) {
				Console::Error("Material did not specify a shader");
				goto failed;
			}
			shaderProgramRid = Engine::Get()->assets->PathToRid(shaderProgramPath);
			if(shaderProgramRid == RID_NONE) {
				Console::Error("Could not resolve rid from shader program path: %s", shaderProgramPath);
				goto failed;
			}

		
			shaderProgram = res->shaderProgramManager->Load(shaderProgramRid);
			if(shaderProgram == HANDLE_NONE) {
				goto failed;
			}
			shader = ShaderProgramManager::Get(shaderProgram);

		
			Document::Node *n = doc.root->Resolve("texture_set");
			if(n != nullptr) {
				Rid textureSetRids[TextureSet::MAX_TEXTURES_PER_SET];
				uint32 textureSetSize = n->count;
				assert(textureSetSize < TextureSet::MAX_TEXTURES_PER_SET);
				for(uint32 i = 0; i < n->count; i++) {
					Rid texRid = eng->assets->PathToRid(n->children[i]->value);
					if(texRid == RID_NONE) {
						Console::Error("Could not resolve rid from texture path: %s", n->children[i]->value);
						goto failed;
					}
					textureSetRids[i] = texRid;
				}
				textureSet = res->textureSetManager->Load(textureSetSize, textureSetRids);
				if(textureSet == HANDLE_NONE) {
					Console::Error("Failed to construct texture set from rid list");
					goto failed;
				}
			}


			n = doc.root->Resolve("uniforms");
			if(n != nullptr) {
				for(uint32 i = 0; i < n->count; i++) {
					Document::Node *uniform = n->children[i];
					if(uniform->count != 1) {
						Console::Error("Uniform node must have a single child specifying data type");
						goto failed;
					}
					Document::Node *dataType = uniform->children[0];
					if(dataType->count == 0) {
						Console::Error("Uniform data type node must have at least one child");
						goto failed;
					}

					uint32 valueCount = dataType->count;
					char *buffer = (char *)Allocator::Malloc(valueCount*4);
					for(uint32 i = 0; i < valueCount; i++) {
						if(dataType->value[0] == 'f') {
							((float *)buffer)[i] = dataType->children[i]->ValueAsFloat();
						} else if(dataType->value[0] == 'u') {
							((uint32 *)buffer)[i] = (uint32)dataType->children[i]->ValueAsInt();
						} else if(dataType->value[0] == 'i') {
							((int32 *)buffer)[i] = (int32)dataType->children[i]->ValueAsInt();
						} else {
							SAFE_DELETE_ARRAY(buffer);
							Console::Error("Unrecognized uniform data type: %s", dataType->value);
							goto failed;
						}
					}
					if(PushConstant(uniform->value, valueCount*4, buffer) == -1) {
						Console::Warning("Warning, material has unbound uniforms <rid %d>", rid);
					}
				}
			}

			this->rid = rid;
			return true;

		failed:
			TextureSetManager::Free(textureSet);
			ShaderProgramManager::Free(shaderProgram);
			return false;
		}


	} // namespace Core

} // namespace Maki