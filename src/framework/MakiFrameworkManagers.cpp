#include "framework/framework_stdafx.h"
#include "framework/MakiFrameworkManagers.h"
#include "core/MakiEngine.h"
#include "core/MakiConfig.h"
#include "framework/MakiScriptManager.h"

namespace Maki
{
	namespace Framework
	{

		FrameworkManagers::FrameworkManagers()
			: PseudoSingleton<FrameworkManagers>(),
			scriptManager(nullptr)
		{
		}

		FrameworkManagers::FrameworkManagers(const Config *config)
			: PseudoSingleton<FrameworkManagers>(),
			scriptManager(nullptr)
		{
			scriptManager = new ScriptManager(config->GetUint("engine.max_scripts", ScriptManager::DEFAULT_SIZE));
		}

		FrameworkManagers::~FrameworkManagers()
		{
			SAFE_DELETE(scriptManager);
		}

		void FrameworkManagers::DumpManagerStats()
		{
			scriptManager->DumpStats("Scripts         ");
		}

		void FrameworkManagers::ReloadAssets()
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			Console::Info("Reloading all hot-swappable assets");
			scriptManager->ReloadAssets();
			DumpManagerStats();
		}

		void FrameworkManagers::ReloadAsset(Rid rid)
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			if(scriptManager->ReloadAsset(rid)) {
			} else {
				Console::Info("Rid was not a hot-swappable asset <rid %d>", rid);
				return;
			}

			Console::Info("Reloading <rid %d>", rid);
			DumpManagerStats();
		}

	} // namespace Core

} // namespace Maki