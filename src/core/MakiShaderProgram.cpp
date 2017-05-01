#include "core/MakiShaderProgram.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiDocument.h"
#include "core/MakiConsole.h"
#include "core/MakiCoreManagers.h"


namespace maki {
	namespace core {

		const char *shader_program_t::variant_data_key_[shader_program_t::variant_count] = {
			"data",
			"depth_data",
			"shadow_data"
		};

		const char *shader_program_t::variant_meta_key_[shader_program_t::variant_count] = {
			"meta",
			"depth_meta",
			"shadow_meta"
		};


		shader_program_t::shader_program_t() {
			for(uint32_t i = 0; i < variant_count - 1; i++) {
				variants[i] = HANDLE_NONE;
			}
		}

		shader_program_t::~shader_program_t() {
			shader_program_manager_t::free(variant_count - 1, variants);
			engine_t::get()->renderer_->delete_shader_program(this);
		}

		bool shader_program_t::load(rid_t rid, variant_t variant) {
			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Failed to parse shader as hdoc");
				return false;
			}

			auto *input_attribute_count_node = doc.root().resolve("input_attribute_count.#0");
			if(input_attribute_count_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires an input_attribute_count node.  <rid %ull>", rid);
				return false;
			}
			input_attribute_count = input_attribute_count_node->value_as_uint(0);
		
			auto *vertex_shader_node = doc.root().resolve("vertex_shader");
			if(vertex_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a vertex_shader node.  <rid %ull>", rid);
				return false;
			}
			if(!vertex_shader.init(vertex_shader_node, variant_data_key_[variant], variant_meta_key_[variant])) {
				if(variant == variant_normal) {
					console_t::error("Failed to load vertex shader <rid %ull>", rid);
				}
				return false;
			}

			auto *pixel_shader_node = doc.root().resolve("pixel_shader");
			if(pixel_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a pixel_shader node.  <rid %ull>", rid);
				return false;
			}
			if(!pixel_shader.init(pixel_shader_node, variant_data_key_[variant], variant_meta_key_[variant])) {
				if(variant == variant_normal) {
					console_t::error("Failed to load pixel shader <rid %ull>", rid);
				}
				return false;
			}

			// Regular shader is responsible for loading up variants_
			this->variant = variant;
			this->rid_ = rid;
			if(variant == variant_normal) {
				auto *res = core_managers_t::get();
				for(uint32_t i = 0; i < variant_count - 1; i++) {
					variants[i] = res->shader_program_manager_->load(rid, (variant_t)(i+1));
				}
			}

			return true;
		}

	} // namespace core
} // namespace maki
