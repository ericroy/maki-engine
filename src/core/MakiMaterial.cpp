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
			uniform_count_(0),
			texture_set_(HANDLE_NONE),
			shader_program_(HANDLE_NONE)
		{
		}

		material_t::material_t(const move_token_t<material_t> &other)
			: resource_t((const move_token_t<resource_t> &)other),
			uniform_count_(other.obj_->uniform_count_),
			texture_set_(other.obj_->texture_set_),
			shader_program_(other.obj_->shader_program_)
		{
			other.obj_->uniform_count_ = 0;
			other.obj_->texture_set_ = HANDLE_NONE;
			other.obj_->shader_program_ = HANDLE_NONE;
			memcpy(uniform_values_, other.obj_->uniform_values_, sizeof(uniform_value_t)*uniform_count_);
			memset(other.obj_->uniform_values_, 0, sizeof(uniform_value_t)*max_uniforms_);
		}

		material_t::material_t(const material_t &other)
		:	resource_t(other),
			uniform_count_(other.uniform_count_),
			texture_set_(other.texture_set_),
			shader_program_(other.shader_program_)
		{
			texture_set_manager_t::add_ref(texture_set_);
			shader_program_manager_t::add_ref(shader_program_);
		
			memcpy(uniform_values_, other.uniform_values_, sizeof(uniform_value_t)*uniform_count_);
			for(uint32 i = 0; i < uniform_count_; i++) {
				uniform_value_t &val = uniform_values_[i];
				char *oldData = val.data_;
				val.data_ = (char *)allocator_t::malloc(val.bytes_);
				memcpy(val.data_, oldData, val.bytes_);
			}
		}

		material_t::~material_t()
		{
			shader_program_manager_t::free(shader_program_);
			texture_set_manager_t::free(texture_set_);
		}

		void material_t::set_shader_program(rid_t shaderRid)
		{
			handle_t newShaderProgram = core_managers_t::get()->shader_program_manager_->load(shaderRid);
			shader_program_manager_t::free(shader_program_);
			shader_program_ = newShaderProgram;
		}
	
		void material_t::set_textures(uint8 count, rid_t *texture_rids)
		{
			handle_t newTextureSet = core_managers_t::get()->texture_set_manager_->load(count, texture_rids);
			texture_set_manager_t::free(texture_set_);
			texture_set_ = newTextureSet;
		}

		int32 material_t::push_constant(const char *key, uint32 bytes, char *data)
		{
			if(shader_program_ == HANDLE_NONE) {
				console_t::warning("Cannot push constant, must set shader program first");
				return -1;
			}
			if(uniform_count_ >= max_uniforms_) {
				console_t::error("Cannot push constant, limit of %d constants reached", max_uniforms_);
				return -1;
			}

			shader_program_t *shader = shader_program_manager_t::get(shader_program_);

			// find constant in vertex shader
			int32 vs_location = shader->vertex_shader_.find_material_constant_location(key);
			int32 ps_location = shader->pixel_shader_.find_material_constant_location(key);

			if(vs_location == -1 && ps_location == -1) {
				console_t::error("Constant not found in vertex or pixel shader: %s", key);
				return -1;
			}

			int32 index = uniform_count_++;
			uniform_value_t &value = uniform_values_[index];
			value.ps_location_ = ps_location;
			value.vs_location_ = vs_location;
			value.bytes_ = bytes;
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

			char *shader_program_path = doc.root_->resolve_value("shader.#0");
			if(shader_program_path == nullptr) {
				console_t::error("material_t did not specify a shader");
				goto failed;
			}
			shader_program_rid = engine_t::get()->assets_->path_to_rid(shader_program_path);
			if(shader_program_rid == RID_NONE) {
				console_t::error("Could not resolve rid from shader program path: %s", shader_program_path);
				goto failed;
			}

		
			shader_program_ = res->shader_program_manager_->load(shader_program_rid);
			if(shader_program_ == HANDLE_NONE) {
				goto failed;
			}
			shader = shader_program_manager_t::get(shader_program_);

		
			n = doc.root_->resolve("texture_set_");
			if(n != nullptr) {
				rid_t textureSetRids[texture_set_t::max_textures_per_set_];
				uint32 textureSetSize = n->count_;
				assert(textureSetSize < texture_set_t::max_textures_per_set_);
				for(uint32 i = 0; i < n->count_; i++) {
					rid_t texRid = eng->assets_->path_to_rid(n->children_[i]->value_);
					if(texRid == RID_NONE) {
						console_t::error("Could not resolve rid from texture path: %s", n->children_[i]->value_);
						goto failed;
					}
					textureSetRids[i] = texRid;
				}
				texture_set_ = res->texture_set_manager_->load(textureSetSize, textureSetRids);
				if(texture_set_ == HANDLE_NONE) {
					console_t::error("Failed to construct texture set from rid list");
					goto failed;
				}
			}


			n = doc.root_->resolve("uniforms");
			if(n != nullptr) {
				for(uint32 i = 0; i < n->count_; i++) {
					document_t::node_t *uniform = n->children_[i];
					if(uniform->count_ != 1) {
						console_t::error("Uniform node must have a single child specifying data type");
						goto failed;
					}
					document_t::node_t *dataType = uniform->children_[0];
					if(dataType->count_ == 0) {
						console_t::error("Uniform data type node must have at least one child");
						goto failed;
					}

					uint32 valueCount = dataType->count_;
					char *buffer = (char *)allocator_t::malloc(valueCount*4);
					for(uint32 i = 0; i < valueCount; i++) {
						if(dataType->value_[0] == 'f') {
							((float *)buffer)[i] = dataType->children_[i]->value_as_float();
						} else if(dataType->value_[0] == 'u') {
							((uint32 *)buffer)[i] = (uint32)dataType->children_[i]->value_as_int();
						} else if(dataType->value_[0] == 'i') {
							((int32 *)buffer)[i] = (int32)dataType->children_[i]->value_as_int();
						} else {
							MAKI_SAFE_DELETE_ARRAY(buffer);
							console_t::error("Unrecognized uniform data type: %s", dataType->value_);
							goto failed;
						}
					}
					if(push_constant(uniform->value_, valueCount*4, buffer) == -1) {
						console_t::warning("warning, material has unbound uniforms <rid %d>", rid);
					}
				}
			}

			this->rid_ = rid;
			return true;

		failed:
			texture_set_manager_t::free(texture_set_);
			shader_program_manager_t::free(shader_program_);
			return false;
		}


	} // namespace core

} // namespace maki
