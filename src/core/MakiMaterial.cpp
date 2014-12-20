#include "core/core_stdafx.h"
#include "core/MakiMaterial.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureSet.h"
#include "core/MakiDocument.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace maki
{
	namespace core
	{

		material_t::material_t()
			: resource_t(),
			uniform_count(0),
			texture_set(HANDLE_NONE),
			shader_program(HANDLE_NONE)
		{
		}

		material_t::material_t(const move_token_t<material_t> &other)
			: resource_t((const move_token_t<resource_t> &)other),
			uniform_count(other.obj->uniform_count_),
			texture_set(other.obj->texture_set_),
			shader_program(other.obj->shader_program_)
		{
			other.obj->uniform_count_ = 0;
			other.obj->texture_set_ = HANDLE_NONE;
			other.obj->shader_program_ = HANDLE_NONE;
			memcpy(uniform_values_, other.obj->uniform_values_, sizeof(uniform_value_t)*uniform_count);
			memset(other.obj->uniform_values_, 0, sizeof(uniform_value_t)*max_uniforms_);
		}

		material_t::material_t(const material_t &other)
		:	resource_t(other),
			uniform_count(other.uniform_count),
			texture_set(other.texture_set_),
			shader_program(other.shader_program_)
		{
			texture_set_manager_t::add_ref(texture_set);
			shader_program_manager_t::add_ref(shader_program);
		
			memcpy(uniform_values_, other.uniform_values_, sizeof(uniform_value_t)*uniform_count);
			for(uint32 i = 0; i < uniform_count; i++) {
				uniform_value_t &val = uniform_values_[i];
				char *oldData = val.data_;
				val.data_ = (char *)allocator_t::malloc(val.bytes);
				memcpy(val.data_, oldData, val.bytes);
			}
		}

		material_t::~material_t()
		{
			shader_program_manager_t::free(shader_program);
			texture_set_manager_t::free(texture_set);
		}

		void material_t::set_shader_program(rid_t shaderRid)
		{
			handle_t newShaderProgram = core_managers_t::get()->shader_program_manager_->load(shaderRid);
			shader_program_manager_t::free(shader_program);
			shader_program = newShaderProgram;
		}
	
		void material_t::set_textures(uint8 count, rid_t *textureRids)
		{
			handle_t newTextureSet = core_managers_t::get()->texture_set_manager_->load(count, textureRids);
			texture_set_manager_t::free(texture_set);
			texture_set = newTextureSet;
		}

		int32 material_t::push_constant(const char *key, uint32 bytes, char *data)
		{
			if(shader_program == HANDLE_NONE) {
				console_t::warning("Cannot push constant, must set shader program first");
				return -1;
			}
			if(uniform_count >= max_uniforms_) {
				console_t::error("Cannot push constant, limit of %d constants reached", max_uniforms_);
				return -1;
			}

			shader_program_t *shader = shader_program_manager_t::get(shader_program);

			// find constant in vertex shader
			int32 vsLocation = shader->vertex_shader_.find_material_constant_location(key);
			int32 psLocation = shader->pixel_shader_.find_material_constant_location(key);

			if(vsLocation == -1 && psLocation == -1) {
				console_t::error("Constant not found in vertex or pixel shader: %s", key);
				return -1;
			}

			int32 index = uniform_count++;
			uniform_value_t &value = uniform_values_[index];
			value.psLocation = psLocation;
			value.vsLocation = vsLocation;
			value.bytes = bytes;
			value.data_ = data;
			return index;
		}

		bool material_t::load(rid_t rid)
		{
			core_managers_t *res = core_managers_t::get();
			engine_t *eng = engine_t::get();

			if(rid == RID_NONE) {
				console_t::error("Failed to load material, rid is RID_NONE");
				return false;
			}

			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Failed to parse material file <rid %d>", rid);
				return false;
			}

			shader_program_t *shader = nullptr;
			rid_t shader_program_rid = RID_NONE;
			document_t::node_t *n = nullptr;

			char *shaderProgramPath = doc.root->resolve_value("shader.#0");
			if(shaderProgramPath == nullptr) {
				console_t::error("material_t did not specify a shader");
				goto failed;
			}
			shader_program_rid = engine_t::get()->assets->path_to_rid(shaderProgramPath);
			if(shader_program_rid == RID_NONE) {
				console_t::error("Could not resolve rid from shader program path: %s", shaderProgramPath);
				goto failed;
			}

		
			shader_program = res->shader_program_manager_->load(shader_program_rid);
			if(shader_program == HANDLE_NONE) {
				goto failed;
			}
			shader = shader_program_manager_t::get(shader_program);

		
			n = doc.root->resolve("texture_set");
			if(n != nullptr) {
				rid_t textureSetRids[texture_set_t::MAX_TEXTURES_PER_SET];
				uint32 textureSetSize = n->count;
				assert(textureSetSize < texture_set_t::MAX_TEXTURES_PER_SET);
				for(uint32 i = 0; i < n->count; i++) {
					rid_t texRid = eng->assets->path_to_rid(n->children[i]->value);
					if(texRid == RID_NONE) {
						console_t::error("Could not resolve rid from texture path: %s", n->children[i]->value);
						goto failed;
					}
					textureSetRids[i] = texRid;
				}
				texture_set = res->texture_set_manager_->load(textureSetSize, textureSetRids);
				if(texture_set == HANDLE_NONE) {
					console_t::error("Failed to construct texture set from rid list");
					goto failed;
				}
			}


			n = doc.root->resolve("uniforms");
			if(n != nullptr) {
				for(uint32 i = 0; i < n->count; i++) {
					document_t::node_t *uniform = n->children[i];
					if(uniform->count != 1) {
						console_t::error("Uniform node must have a single child specifying data type");
						goto failed;
					}
					document_t::node_t *dataType = uniform->children[0];
					if(dataType->count == 0) {
						console_t::error("Uniform data type node must have at least one child");
						goto failed;
					}

					uint32 valueCount = dataType->count;
					char *buffer = (char *)allocator_t::malloc(valueCount*4);
					for(uint32 i = 0; i < valueCount; i++) {
						if(dataType->value[0] == 'f') {
							((float *)buffer)[i] = dataType->children[i]->value_as_float();
						} else if(dataType->value[0] == 'u') {
							((uint32 *)buffer)[i] = (uint32)dataType->children[i]->value_as_int();
						} else if(dataType->value[0] == 'i') {
							((int32 *)buffer)[i] = (int32)dataType->children[i]->value_as_int();
						} else {
							MAKI_SAFE_DELETE_ARRAY(buffer);
							console_t::error("Unrecognized uniform data type: %s", dataType->value);
							goto failed;
						}
					}
					if(push_constant(uniform->value, valueCount*4, buffer) == -1) {
						console_t::warning("warning, material has unbound uniforms <rid %d>", rid);
					}
				}
			}

			this->rid_ = rid;
			return true;

		failed:
			texture_set_manager_t::free(texture_set);
			shader_program_manager_t::free(shader_program);
			return false;
		}


	} // namespace core

} // namespace maki
