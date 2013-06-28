#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiManager.h"
#include "framework/MakiScript.h"

namespace Maki
{
	namespace Framework
	{

		class ScriptManager : public Core::Manager<Script, ScriptManager>
		{
		public:
			static const int32 DEFAULT_SIZE = 32;

		public:
			ScriptManager(uint32 size = DEFAULT_SIZE);	
			virtual ~ScriptManager();
			Handle Load(Rid rid);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki