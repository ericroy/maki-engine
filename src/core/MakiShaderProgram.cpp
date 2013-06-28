#include "core/core_stdafx.h"
#include "core/MakiShaderProgram.h"
#include "core/MakiEngine.h"
#include "core/MakiCoreManagers.h"
#include "core/MakiRenderer.h"
#include "core/MakiShaderProgramManager.h"


namespace Maki
{
	namespace Core
	{

		char *ShaderProgram::variantDataKey[VariantCount] = {
			"data",
			"depth_data",
			"shadow_data"
		};
		char *ShaderProgram::variantMetaKey[VariantCount] = {
			"meta",
			"depth_meta",
			"shadow_meta"
		};


		ShaderProgram::ShaderProgram()
			: vertexShader(), pixelShader(), variant(Variant_Normal)
		{
			for(uint32 i = 0; i < VariantCount-1; i++) {
				variants[i] = HANDLE_NONE;
			}
		}

		ShaderProgram::~ShaderProgram() {
			ShaderProgramManager::Free(VariantCount-1, variants);
			Engine::Get()->renderer->DeleteShaderProgram(this);
		}

		bool ShaderProgram::Load(Rid rid, Variant variant)
		{
			Document doc;
			if(!doc.Load(rid)) {
				Console::Error("Failed to parse shader as hdoc");
				return false;
			}
		
			Document::Node *vertexShaderNode = doc.root->Resolve("vertex_shader");
			if(!vertexShaderNode) {
				Console::Error("Failed to parse shader. Shader doc requires a vertex_shader node.  <rid %d>", rid);
				return false;
			}
			if(!vertexShader.Init(vertexShaderNode, variantDataKey[variant], variantMetaKey[variant])) {
				if(variant == Variant_Normal) {
					Console::Error("Failed to load vertex shader <rid %d>", rid);
				}
				return false;
			}

			Document::Node *pixelShaderNode = doc.root->Resolve("pixel_shader");
			if(!pixelShaderNode) {
				Console::Error("Failed to parse shader. Shader doc requires a pixel_shader node.  <rid %d>", rid);
				return false;
			}
			if(!pixelShader.Init(pixelShaderNode, variantDataKey[variant], variantMetaKey[variant])) {
				if(variant == Variant_Normal) {
					Console::Error("Failed to load pixel shader <rid %d>", rid);
				}
				return false;
			}

			// Regular shader is responsible for loading up variants
			this->variant = variant;
			this->rid = rid;
			if(variant == Variant_Normal) {
				CoreManagers *res = CoreManagers::Get();
				for(uint32 i = 0; i < VariantCount-1; i++) {
					variants[i] = res->shaderProgramManager->Load(rid, (Variant)(i+1));
				}
			}

			return true;
		}

	} // namespace Core

} // namespace Maki