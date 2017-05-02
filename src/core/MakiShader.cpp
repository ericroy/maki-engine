#include <sstream>
#include "core/MakiShader.h"
#include "core/MakiEngine.h"
#include "core/MakiBase64.h"
#include "core/MakiConsole.h"

using namespace std;

namespace maki {
	namespace core {

		namespace {
			const char *frame_uniform_names[frame_uniform_max + 1] = {
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
		
			const char *object_uniform_names[object_uniform_max + 1] = {
				"uModel",
				"uModelView",
				"uModelViewProjection"
			};
		}

		frame_uniform_t get_frame_uniform_by_name(const char *name) {
			for(int32_t i = 0; i <= frame_uniform_max; i++) {
				if(strcmp(frame_uniform_names[i], name) == 0)
					return (frame_uniform_t)i;
			}
			return frame_uniform_none;
		}

		object_uniform_t get_object_uniform_by_name(const char *name) {
			for(int32_t i = 0; i <= object_uniform_max; i++) {
				if(strcmp(object_uniform_names[i], name) == 0)
					return (object_uniform_t)i;
			}
			return object_uniform_none;
		}


		material_uniform_location_t::material_uniform_location_t(int32_t location, char *name) : location(location) {
			strncpy(this->name, name, sizeof(this->name));
			this->name[sizeof(this->name) - 1] = 0;
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

			stringstream in(encoded_data);
			stringstream out(ios::out | ios::binary);
			if(!base64::decode(in, out)) {
				console_t::error("Failed to base64 decode shader program");
				return false;
			}
			string data = out.str();
			program_data.set_length(data.length());
			memcpy(program_data.data(), data.c_str(), data.length());

			auto *meta_node = shader_node.resolve(meta_key);
			if(meta_node == nullptr)
				return true;

			auto *engine_per_frame = meta_node->resolve("engine_per_frame");
			if(engine_per_frame != nullptr) {
				if(!engine_per_frame->resolve_as_int("slot.#0", &frame_uniform_buffer_location_))
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
					engine_frame_uniform_locations_[c] = (int32_t)offset;				
					engine_frame_uniform_bytes_ = max(offset + size, engine_frame_uniform_bytes_);
				}		
			}

			auto *engine_per_object = meta_node->resolve("engine_per_object");
			if(engine_per_object != nullptr) {
				if(!engine_per_object->resolve_as_int("slot.#0", &object_uniform_buffer_location_))
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
					engine_object_uniform_locations_[c] = (int32_t)offset;				
					engine_object_uniform_bytes_ = max(offset + size, engine_object_uniform_bytes_);
				}		
			}

			auto *material = meta_node->resolve("material");
			if(material != nullptr) {
				if(!material->resolve_as_int("slot.#0", &material_uniform_buffer_location_))
					return false;

				auto *uniforms = material->resolve("uniforms");
				if(!uniforms)
					return false;

				for(const auto &uni : *uniforms) {
					assert(uni.length() == 2 && "uniform must have offset and size");

					uint32_t offset = (uint32_t)uni[0].value_as_uint();
					uint32_t size = (uint32_t)uni[1].value_as_uint();
				
					material_uniform_locations_.push_back(material_uniform_location_t((int32_t)offset, uni.value()));
					material_uniform_bytes_ = max(offset + size, material_uniform_bytes_);
				}		
			}

			return true;
		}

		int32_t shader_t::find_material_constant_location(const char *name) {
			const uint32_t count = material_uniform_locations_.size();
			for(uint32_t i = 0; i < count; i++) {
				if(strcmp(name, material_uniform_locations_[i].name) == 0) {
					return material_uniform_locations_[i].location;
			}
			return -1;
		}


	} // namespace core
} // namespace maki
