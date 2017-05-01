#include "core/MakiShader.h"
#include "core/MakiEngine.h"
#include "core/MakiBase64.h"
#include <sstream>

namespace maki {
	namespace core {

		const char *shader_t::frame_uniform_names[frame_uniform_count] = {
			"uView",
			"uProjection",
			"uViewProjection",
			"uLightPositions",
			"uLightDirections",
			"uLightViewProj",
			"uLightProperties",
			"uShadowMapProperties",
			"uLightSplitRegions",
			"uCameraSplitDistances",
			"uGlobalAmbientColor",
			"uCameraWithHeightNearFar",
		};
	
		const char *shader_t::object_uniform_names[object_uniform_count] = {
			"uModel",
			"uModelView",
			"uModelViewProjection"
		};

		shader_t::material_uniform_location_t::material_uniform_location_t(int32_t location, char *name) : location(location) {
			strcpy(this->name, name);
		}

		shader_t::frame_uniform_t shader_t::get_frame_uniform_by_name(const char *name) {
			for(int32_t i = 0; i < frame_uniform_count; i++) {
				if(strcmp(frame_uniform_names[i], name) == 0)
					return (frame_uniform_t)i;
			}
			return frame_uniform_none;
		}

		shader_t::object_uniform_t shader_t::get_object_uniform_by_name(const char *name) {
			for(int32_t i = 0; i < object_uniform_count; i++) {
				if(strcmp(object_uniform_names[i], name) == 0)
					return (object_uniform_t)i;
			}
			return object_uniform_none;
		}



		shader_t::~shader_t() {
			MAKI_SAFE_FREE(program_data);
		}

		bool shader_t::init(const document_t::node_t &shader_node, const char *data_key, const char *meta_key) {
			auto *data_node = shader_node.resolve(data_key);
			if(data_node == nullptr || data_node->length() != 1)
				return false;

			char *encoded_data = (*data_node)[0].value();
			if(encoded_data == nullptr) {
				console_t::error("Failed to parse shader, no base64 encoded program found in 'data' node");
				return false;
			}

			std::stringstream in(encoded_data);
			std::stringstream out(std::ios::out | std::ios::binary);
			if(!base64::decode(in, out)) {
				console_t::error("Failed to base64 decode shader program");
				return false;
			}
			std::string data = out.str();
			program_data_bytes = data.length();

			MAKI_SAFE_FREE(program_data_);
			program_data = (char *)allocator_t::malloc(program_data_bytes);
			memcpy(program_data, data.c_str(), program_data_bytes);

			auto *meta_node = shader_node.resolve(meta_key);
			if(meta_node == nullptr)
				return true;

			auto *engine_per_frame = meta_node->resolve("engine_per_frame");
			if(engine_per_frame != nullptr) {
				if(!engine_per_frame->resolve_as_int("slot.#0", &frame_uniform_buffer_location))
					return false;

				auto *uniforms = engine_per_frame->resolve("uniforms");
				if(!uniforms)
					return false;

				for(const auto &uni : *uniforms) {
					assert(uni.length() == 2 && "uniform must have offset and size");

					frame_uniform_t c = get_frame_uniform_by_name(uni.value());
					if(c == frame_uniform_none)
						return false;

					uint32_t offset = (uint32_t)uni[0].>value_as_uint();
					uint32_t size = (uint32_t)uni[1].>value_as_uint();
					engine_frame_uniform_locations[c] = (int32_t)offset;				
					engine_frame_uniform_bytes = std::max(offset + size, engine_frame_uniform_bytes);
				}		
			}

			auto *engine_per_object = meta_node->resolve("engine_per_object");
			if(engine_per_object != nullptr) {
				if(!engine_per_object->resolve_as_int("slot.#0", &object_uniform_buffer_location))
					return false;

				auto *uniforms = engine_per_object->resolve("uniforms");
				if(!uniforms)
					return false;

				for(const auto &uni : *uniforms) {
					assert(uni.length() == 2 && "uniform must have offset and size");

					object_uniform_t c = get_object_uniform_by_name(uni.value());
					if(c == object_uniform_none)
						return false;

					uint32_t offset = (uint32_t)uni[0].value_as_uint();
					uint32_t size = (uint32_t)uni[1].value_as_uint();
					engine_object_uniform_locations[c] = (int32_t)offset;				
					engine_object_uniform_bytes = std::max(offset + size, engine_object_uniform_bytes);
				}		
			}

			auto *material = meta_node->resolve("material");
			if(material != nullptr) {
				if(!material->resolve_as_int("slot.#0", &material_uniform_buffer_location))
					return false;

				auto *uniforms = material->resolve("uniforms");
				if(!uniforms)
					return false;

				for(const auto &uni : *uniforms) {
					assert(uni.length() == 2 && "uniform must have offset and size");

					uint32_t offset = (uint32_t)uni[0].value_as_uint();
					uint32_t size = (uint32_t)uni[1].value_as_uint();
				
					material_uniform_locations.push_back(material_uniform_location_t((int32_t)offset, uni.value()));
					material_uniform_bytes = std::max(offset + size, material_uniform_bytes);
				}		
			}

			return true;
		}

		int32_t shader_t::find_material_constant_location(const char *name) {
			const uint32_t count = material_uniform_locations.size();
			for(uint32_t i = 0; i < count; i++) {
				if(strcmp(name, material_uniform_locations[i].name) == 0) {
					return material_uniform_locations[i].location;
			}
			return -1;
		}


	} // namespace core
} // namespace maki
