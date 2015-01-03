#include "framework/framework_stdafx.h"
#include "framework/MakiFrameworkManagers.h"
#include "core/MakiEngine.h"
#include "core/MakiConfig.h"
#include "framework/MakiScriptManager.h"
#include "framework/MakiFlashMovieManager.h"

namespace Maki
{
	namespace Framework
	{

		FrameworkManagers::FrameworkManagers()
			: PseudoSingleton<FrameworkManagers>(),
			scriptManager(nullptr),
			flashMovieManager(nullptr)
		{
		}

		FrameworkManagers::FrameworkManagers(const Config *config)
			: PseudoSingleton<FrameworkManagers>(),
			scriptManager(nullptr),
			flashMovieManager(nullptr)
		{
			scriptManager = new ScriptManager(config->GetUint("engine.max_scripts", ScriptManager::DEFAULT_SIZE));
			flashMovieManager = new FlashMovieManager(config->GetUint("engine.max_flash_movies", FlashMovieManager::DEFAULT_SIZE));
		}

		FrameworkManagers::~FrameworkManagers()
		{
			SAFE_DELETE(scriptManager);
			SAFE_DELETE(flashMovieManager);
		}

		void FrameworkManagers::DumpManagerStats()
		{
			scriptManager->DumpStats		("Scripts         ");
			flashMovieManager->DumpStats	("FlashMovies     ");
		}

		void FrameworkManagers::ReloadAssets()
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			Console::Info("Reloading all hot-swappable assets");
			scriptManager->ReloadAssets();
			flashMovieManager->ReloadAssets();
			DumpManagerStats();
		}

		bool FrameworkManagers::ReloadAsset(Rid rid)
		{
			Engine *eng = Engine::Get();
			if(eng != nullptr) {
				eng->renderer->SyncWithCore();
			}

			// Caution, relies on short-circuit evaluation
			if(	scriptManager->ReloadAsset(rid) ||
				flashMovieManager->ReloadAsset(rid)
				) {
				return true;
			}
			return false;
		}

	} // namespace Core

} // namespace Maki