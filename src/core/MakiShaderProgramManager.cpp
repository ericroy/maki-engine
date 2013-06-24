#include "core/core_stdafx.h"
#include "core/MakiShaderProgramManager.h"
#include "core/MakiEngine.h"
#include "core/MakiRenderer.h"

namespace Maki
{
	namespace Core
	{

		ShaderProgramManager::ShaderProgramManager(uint32 size)
			: Manager<ShaderProgram, ShaderProgramManager>(size)
		{
			assert(size <= (1<<DrawCommand::BITS_PER_SHADER_PROGRAM) && "ShaderProgramManager too large, add more bits in DrawCommand");
		}
	
		ShaderProgramManager::~ShaderProgramManager()
		{
		}

		Handle ShaderProgramManager::Load(Rid rid, ShaderProgram::Variant variant)
		{
			Handle handle = resPool->Match(FindPredicate(rid, variant)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			ShaderProgram *shader = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(shader) ShaderProgram();
		
			if(!shader->Load(rid, variant)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}

			Engine *eng = Engine::Get();
			if(!eng->renderer->CreateShaderProgram(shader)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}

			return handle;
		}

		void ShaderProgramManager::ReloadAssets()
		{
			Engine *eng = Engine::Get();
			const ResourcePool<ShaderProgram>::Iterator end = resPool->End();
			for(ResourcePool<ShaderProgram>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				ShaderProgram *shader = iter.Ptr();
				Rid rid = shader->rid;
				ShaderProgram::Variant variant = shader->variant;

				if(rid != RID_NONE) {
					shader->~ShaderProgram();
					new(shader) ShaderProgram();
					if(shader->Load(rid, variant)) {
						eng->renderer->CreateShaderProgram(shader);
					}
				}
			}
		}
	
		bool ShaderProgramManager::ReloadAsset(Rid rid)
		{
			bool found = false;
			for(uint32 variant = ShaderProgram::Variant_Normal; variant < ShaderProgram::VariantCount; variant++) {
				Handle handle = resPool->Match(FindPredicate(rid, (ShaderProgram::Variant)variant)) | managerId;
				if(handle == HANDLE_NONE) {
					continue;
				}
				found = true;

				ShaderProgram *shader = resPool->Get(handle & HANDLE_VALUE_MASK);
				resPool->Free(handle & HANDLE_VALUE_MASK);

				if(rid != RID_NONE) {
					shader->~ShaderProgram();
					new(shader) ShaderProgram();
					if(shader->Load(rid, (ShaderProgram::Variant)variant)) {
						Engine::Get()->renderer->CreateShaderProgram(shader);
					}
				}
			}
			return found;
		}

	} // namespace Core

} // namespace Maki