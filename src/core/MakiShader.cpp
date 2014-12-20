#include "core/core_stdafx.h"
#include "core/MakiShader.h"
#include "core/MakiEngine.h"
#include "core/MakiBase64.h"
#include <sstream>

namespace maki
{
	namespace core
	{

		const char *shader_t::frame_uniform_names_[frame_uniform_count_] = {
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
	
		const char *shader_t::object_uniform_names_[object_uniform_count_] = {
			"uModel",
			"uModelView",
			"uModelViewProjection"
		};

		shader_t::material_uniform_location_t::material_uniform_location_t()
			: location_(-1)
		{
			name_[0] = 0;
		}

		shader_t::material_uniform_location_t::material_uniform_location_t(int32 location, char *name)
			: location_(location)
		{
			strcpy(name_, name);
		}

		shader_t::frame_uniform_t shader_t::get_frame_uniform_by_name(const char *name)
		{
			for(int32 i = 0; i < frame_uniform_count_; i++) {
				if(strcmp(frame_uniform_names_[i], name) == 0) {
					return (frame_uniform_t)i;
				}
			}
			return frame_uniform_none_;
		}

		shader_t::object_uniform_t shader_t::get_object_uniform_by_name(const char *name)
		{
			for(int32 i = 0; i < object_uniform_count_; i++) {
				if(strcmp(object_uniform_names_[i], name) == 0) {
					return (object_uniform_t)i;
				}
			}
			return object_uniform_none_;
		}



		shader_t::shader_t()
			: handle_(0),
			program_data_(nullptr),
			program_data_bytes_(0),
			engine_frame_uniform_bytes_(0),
			engine_object_uniform_bytes_(0),
			material_uniform_bytes_(0),
			frame_uniform_buffer_location_(-1),
			object_uniform_buffer_location_(-1),
			material_uniform_buffer_location_(-1)
		{
			memset(engine_frame_uniform_locations_, -1, sizeof(engine_frame_uniform_locations_));
			memset(engine_object_uniform_locations_, -1, sizeof(engine_object_uniform_locations_));
		}

		shader_t::~shader_t()
		{
			MAKI_SAFE_FREE(program_data_);
		}

		bool shader_t::init(document_t::node_t *shader_node, const char *data_key, const char *meta_key)
		{
			document_t::node_t *dataNode = shader_node->resolve(data_key);
			if(dataNode == nullptr || dataNode->count_ != 1) {
				return false;
			}

			char *encodedData = dataNode->children_[0]->value_;
			if(encodedData == nullptr) {
				console_t::error("Failed to parse shader, no base64 encoded program found in 'data' node");
				return false;
			}

			std::stringstream in(encodedData);
			std::stringstream out(std::ios::out | std::ios::binary);
			if(!base64::decode(in, out)) {
				console_t::error("Failed to base64 decode shader program");
				return false;
			}
			std::string data = out.str();
			program_data_bytes_ = data.length();

			MAKI_SAFE_FREE(program_data_);
			program_data_ = (char *)allocator_t::malloc(program_data_bytes_);
			memcpy(program_data_, data.c_str(), program_data_bytes_);
		

			document_t::node_t *meta_node = shader_node->resolve(meta_key);
			if(meta_node == nullptr) {
				return true;
			}

			document_t::node_t *engine_per_frame = meta_node->resolve("engine_per_frame");
			if(engine_per_frame != nullptr) {
				if(!engine_per_frame->resolve_as_int("slot.#0", &frame_uniform_buffer_location_)) {
					return false;
				}

				document_t::node_t *uniforms = engine_per_frame->resolve("uniforms");
				if(!uniforms) {
					return false;
				}

				for(uint32 i = 0; i < uniforms->count_; i++) {
					document_t::node_t *uni = uniforms->children_[i];
					assert(uni->count_ == 2 && "uniform must have offset and size");

					frame_uniform_t c = get_frame_uniform_by_name(uni->value_);
					if(c == frame_uniform_none_) {
						return false;
					}
					uint32 offset = uni->children_[0]->value_as_uint();
					uint32 size = uni->children_[1]->value_as_uint();
					engine_frame_uniform_locations_[c] = (int32)offset;				
					engine_frame_uniform_bytes_ = std::max(offset+size, engine_frame_uniform_bytes_);
				}		
			}

			document_t::node_t *engine_per_object = meta_node->resolve("engine_per_object");
			if(engine_per_object != nullptr) {
				if(!engine_per_object->resolve_as_int("slot.#0", &object_uniform_buffer_location_)) {
					return false;
				}

				document_t::node_t *uniforms = engine_per_object->resolve("uniforms");
				if(!uniforms) {
					return false;
				}

				for(uint32 i = 0; i < uniforms->count_; i++) {
					document_t::node_t *uni = uniforms->children_[i];
					assert(uni->count_ == 2 && "uniform must have offset and size");

					object_uniform_t c = get_object_uniform_by_name(uni->value_);
					if(c == object_uniform_none_) {
						return false;
					}
					uint32 offset = uni->children_[0]->value_as_uint();
					uint32 size = uni->children_[1]->value_as_uint();
					engine_object_uniform_locations_[c] = (int32)offset;				
					engine_object_uniform_bytes_ = std::max(offset+size, engine_object_uniform_bytes_);
				}		
			}

			document_t::node_t *material = meta_node->resolve("material");
			if(material != nullptr) {
				if(!material->resolve_as_int("slot.#0", &material_uniform_buffer_location_)) {
					return false;
				}

				document_t::node_t *uniforms = material->resolve("uniforms");
				if(!uniforms) {
					return false;
				}

				for(uint32 i = 0; i < uniforms->count_; i++) {
					document_t::node_t *uni = uniforms->children_[i];
					assert(uni->count_ == 2 && "uniform must have offset and size");

					uint32 offset = uni->children_[0]->value_as_uint();
					uint32 size = uni->children_[1]->value_as_uint();
				
					material_uniform_locations_.push_back(material_uniform_location_t((int32)offset, uni->value_));
					material_uniform_bytes_ = std::max(offset+size, material_uniform_bytes_);
				}		
			}

			return true;
		}

		int32 shader_t::find_material_constant_location(const char *name)
		{
			const uint32 count = material_uniform_locations_.size();
			for(uint32 i = 0; i < count; i++) {
				if(strcmp(name, material_uniform_locations_[i].name_) == 0) {
					return material_uniform_locations_[i].location_;
				}
			}
			return -1;
		}


	} // namespace core

} // namespace maki
