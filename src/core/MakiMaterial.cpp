#include "core/MakiMaterial.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiTextureSet.h"
#include "core/MakiConsole.h"
#include "core/MakiDocument.h"
#include "core/MakiAssetLibrary.h"
#include "core/MakiTextureSetManager.h"
#include "core/MakiShaderProgramManager.h"

namespace maki {
	namespace core {

		material_t::material_t(material_t &&other) {
			texture_set_ = move(other.texture_set_);
			shader_program_ = move(other.shader_program_);
			for (int32_t i = 0; i < other.uniform_count_; i++)
				uniform_values_[i] = move(other.uniform_values_[i])
			uniform_count_ = other.uniform_count_;
			other.uniform_count_ = 0;
		}

		int32_t material_t::push_constant(const char *key, const array_t<char> &data) {
			if(!shader_program_) {
				console_t::warning("Cannot push constant, must set shader program first");
				return -1;
			}
			if(uniform_count_ >= max_uniforms) {
				console_t::error("Cannot push constant, limit of %d constants reached", max_uniforms);
				return -1;
			}

			// find constant in vertex shader
			int32_t vs_location = shader_program->vertex_shader.find_material_constant_location(key);
			int32_t ps_location = shader_program->pixel_shader.find_material_constant_location(key);
			if(vs_location == -1 && ps_location == -1) {
				console_t::error("Constant not found in vertex or pixel shader: %s", key);
				return -1;
			}

			int32_t index = uniform_count_++;
			auto &value = uniform_values_[index];
			value.ps_location = ps_location;
			value.vs_location = vs_location;
			value.data = data;
			return index;
		}

		bool material_t::load(rid_t rid) {
			auto *res = core_managers_t::get();
			auto *eng = engine_t::get();

			ref_t<shader_program_t> shader_temp;
			ref_t<texture_set_t> ts_temp;

			if(rid == RID_NONE) {
				console_t::error("Failed to load material, rid is RID_NONE");
				return false;
			}

			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Failed to parse material file <rid %u>", rid);
				return false;
			}

			char *path = doc.root_->resolve_value("shader.#0");
			if(path == nullptr) {
				console_t::error("Material did not specify a shader");
				return false;
			}
			auto rid = engine_t::get()->assets->path_to_rid(path);
			if(rid == RID_NONE) {
				console_t::error("Could not resolve rid from shader program path: %s", path);
				return false;
			}

			shader_temp = res->shader_program_manager->get_or_load(rid);
			if(!shader_temp)
				return false;

			auto  *n = doc.root_->resolve("texture_set");
			if(n != nullptr) {
				rid_t texture_rids[texture_set_t::max_textures_per_set];
				MAKI_ASSERT(n->length() < texture_set_t::max_textures_per_set);
				for(uint32_t i = 0; i < n->length(); i++) {
					rid_t rid = eng->assets->path_to_rid((*n)[i].value());
					if(rid == RID_NONE) {
						console_t::error("Could not resolve rid from texture path: %s", (*n)[i].value());
						return false;
					}
					texture_rids[i] = rid;
				}
				ts_temp = res->texture_set_manager_->get_or_load(n->length(), texture_rids);
				if(!ts_temp) {
					console_t::error("Failed to construct texture set from rid list");
					return false;
				}
			}


			n = doc.root_->resolve("uniforms");
			if(n != nullptr) {
				for(uint32_t i = 0; i < n->length(); i++) {
					document_t::node_t *uniform = (*n)[i];
					if(uniform->length() != 1) {
						console_t::error("Uniform node must have a single child specifying data type");
						return false;
					}
					document_t::node_t *data_type = (*uniform)[0];
					if(data_type->length() == 0) {
						console_t::error("Uniform data type node must have at least one child");
						return false;
					}

					uint32_t value_count = data_type->length();
					array_t<char> buffer(value_count * 4);
					for(uint32_t i = 0; i < value_count; i++) {
						if(data_type->value()[0] == 'f') {
							((float *)buffer.data())[i] = (*data_type)[i].value_as_float();
						} else if(data_type->value()[0] == 'u') {
							((uint32_t *)buffer.data())[i] = (uint32_t)(*data_type)[i].value_as_int();
						} else if(data_type->value()[0] == 'i') {
							((int32_t *)buffer.data())[i] = (int32_t)(*data_type)[i].value_as_int();
						} else {
							console_t::error("Unrecognized uniform data type: %s", data_type->value());
							return false;
						}
					}
					if(push_constant(uniform->value(), value_count * 4, buffer) == -1) {
						console_t::warning("warning, material has unbound uniforms <rid %u>", rid);
					}
				}
			}

			shader_program_ = move(shader_temp);
			texture_set_ = move(ts_temp);
			rid_ = rid;
			return true;
		}


	} // namespace core
} // namespace maki
