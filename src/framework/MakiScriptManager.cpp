#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScriptManager.h"

namespace Maki
{
	namespace Framework
	{

		ScriptManager::ScriptManager(uint32 size)
			: Manager<Script, ScriptManager>(size, "ScriptManager")
		{
			
		}

		ScriptManager::~ScriptManager()
		{
		}

		Handle ScriptManager::Load(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Script>(rid)) | managerId;
			if(handle != HANDLE_NONE) {
				return handle;
			}

			handle = resPool->Alloc() | managerId;
			Script *s = resPool->Get(handle & HANDLE_VALUE_MASK);
			new(s) Script();
			if(!s->Load(rid)) {
				resPool->Free(handle & HANDLE_VALUE_MASK);
				return HANDLE_NONE;
			}
			return handle;
		}

		void ScriptManager::ReloadAssets()
		{
			Engine *eng = Engine::Get();

			const ResourcePool<Script>::Iterator end = resPool->End();
			for(ResourcePool<Script>::Iterator iter = resPool->Begin(); iter != end; ++iter) {
				Script *s = iter.Ptr();
				Rid rid = s->rid;
				if(rid != RID_NONE) {
					s->~Script();
					new(s) Script();
					s->Load(rid);
				}
			}
		}
		
		bool ScriptManager::ReloadAsset(Rid rid)
		{
			Handle handle = resPool->Match(Resource::FindPredicate<Script>(rid)) | managerId;
			if(handle == HANDLE_NONE) {
				return false;
			}
			Script *s = resPool->Get(handle & HANDLE_VALUE_MASK);
			resPool->Free(handle & HANDLE_VALUE_MASK);

			if(rid != RID_NONE) {
				s->~Script();
				new(s) Script();
				return s->Load(rid);
			}
			return true;
		}
		

	} // namespace Core

} // namespace Maki