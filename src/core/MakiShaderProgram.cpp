#include "core/MakiShaderProgram.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"
#include "core/MakiDocument.h"
#include "core/MakiConsole.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiShaderProgramManager.h"


namespace maki {
	namespace core {

		namespace {
			const char *variant_data_key_[shader_program_variant_max + 1] = {
				"data",
				"depth_data",
				"shadow_data"
			};

			const char *variant_meta_key_[shader_program_variant_max + 1] = {
				"meta",
				"depth_meta",
				"shadow_meta"
			};
		}


		shader_program_t::~shader_program_t() {
			engine_t::get()->renderer->delete_shader_program(this);
		}

		bool shader_program_t::load(rid_t rid, shader_program_variant_t variant) {
			document_t doc;
			if(!doc.load(rid)) {
				console_t::error("Failed to parse shader as hdoc");
				return false;
			}

			auto *input_attribute_count_node = doc.root().resolve("input_attribute_count.#0");
			if(input_attribute_count_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires an input_attribute_count node.  <rid %u>", rid);
				return false;
			}
			input_attribute_count_ = input_attribute_count_node->value_as_uint(0u);
		
			auto *vertex_shader_node = doc.root().resolve("vertex_shader");
			if(vertex_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a vertex_shader node.  <rid %u>", rid);
				return false;
			}
			if(!vertex_shader_.init(*vertex_shader_node, variant_data_key[variant], variant_meta_key[variant])) {
				if(variant == shader_program_variant_normal)
					console_t::error("Failed to load vertex shader <rid %u>", rid);
				return false;
			}

			auto *fragment_shader_node = doc.root().resolve("fragment_shader");
			if(fragment_shader_node == nullptr) {
				console_t::error("Failed to parse shader. Shader doc requires a fragment_shader node.  <rid %u>", rid);
				return false;
			}
			if(!fragment_shader_.init(*fragment_shader_node, variant_data_key[variant], variant_meta_key[variant])) {
				if(variant == shader_program_variant_normal)
					console_t::error("Failed to load fragment shader <rid %u>", rid);
				return false;
			}

			auto *eng = engine_t::get();
			if (!eng->renderer->create_shader_program(this))
				return false;

			// Regular shader is responsible for loading up variants_
			variant_ = variant;
			rid_ = rid;

			if(variant == shader_program_variant_normal) {
				auto *res = core_managers_t::get();
				for(uint32_t i = 0; i < shader_program_variant_max; i++)
					variants_[i] = res->shader_program_manager->load(rid, (shader_program_variant_t)(i + 1));
			}

			return true;
		}

	} // namespace core
} // namespace maki
