#include "core/core_stdafx.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgramManager.h"


namespace maki
{
	namespace core
	{

		const char *shader_program_t::variant_data_key_[variant_count_] = {
			"data",
			"depth_data",
			"shadow_data"
		};
		const char *shader_program_t::variant_meta_key_[variant_count_] = {
			"meta",
			"depth_meta",
			"shadow_meta"
		};


		shader_program_t::shader_program_t()
			: vertex_shader_(), pixel_shader_(), variant_(variant_normal_), handle_(0)
		{
			for(uint32_t i = 0; i < variant_count_-1; i++) {
				variants_[i] = HANDLE_NONE;
			}
		}

		shader_program_t::~shader_program_t() {
			shader_program_manager_t::free(variant_count_-1, variants_);
			engine_t::get()->renderer_->delete_shader_program(this);
		}

		bool shader_program_t::load(rid_t rid, variant_t variant)
		{
			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Failed to parse shader as hdoc");
				return false;
			}

			document_t::node_t *input_attribute_count_node = doc.root_->resolve("input_attribute_count.#0");
			if(input_attribute_count_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires an input_attribute_count node.  <rid %d>", rid);
				return false;
			}
			input_attribute_count_ = input_attribute_count_node->value_as_uint(0U);
		
			document_t::node_t *vertex_shader_node = doc.root_->resolve("vertex_shader");
			if(vertex_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a vertex_shader node.  <rid %d>", rid);
				return false;
			}
			if(!vertex_shader_.init(vertex_shader_node, variant_data_key_[variant], variant_meta_key_[variant])) {
				if(variant == variant_normal_) {
					console_t::error("Failed to load vertex shader <rid %d>", rid);
				}
				return false;
			}

			document_t::node_t *pixel_shader_node = doc.root_->resolve("pixel_shader");
			if(pixel_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a pixel_shader node.  <rid %d>", rid);
				return false;
			}
			if(!pixel_shader_.init(pixel_shader_node, variant_data_key_[variant], variant_meta_key_[variant])) {
				if(variant == variant_normal_) {
					console_t::error("Failed to load pixel shader <rid %d>", rid);
				}
				return false;
			}

			// Regular shader is responsible for loading up variants_
			this->variant_ = variant;
			this->rid_ = rid;
			if(variant == variant_normal_) {
				core_managers_t *res = core_managers_t::get();
				for(uint32_t i = 0; i < variant_count_-1; i++) {
					variants_[i] = res->shader_program_manager_->load(rid, (variant_t)(i+1));
				}
			}

			return true;
		}

	} // namespace core

} // namespace maki
