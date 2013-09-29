#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiFrameworkManagers.h"
#include "framework/MakiFlashMovieManager.h"
#include "framework/components/MakiFlashComponent.h"
#include "framework/MakiComponentPool.h"


namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Flash::Flash()
				: Component(TYPE, DEPENDENCIES),
				movie(HANDLE_NONE)
			{
			}

			Flash::~Flash()
			{
				FlashMovieManager::Free(movie);
			}

			bool Flash::Init(Document::Node *node)
			{
				Engine *eng = Engine::Get();
				FrameworkManagers *res = FrameworkManagers::Get();

				const char *moviePath = node->ResolveValue("#0");
				if(moviePath == nullptr) {
					Console::Error("Flash component did not specify a movie");
					return false;
				}
				Rid movieRid = eng->assets->PathToRid(moviePath);
				if(movieRid == RID_NONE) {
					Console::Error("No RID for path: %s", moviePath);
					return false;
				}

				return Init(movieRid);
			}

			bool Flash::Init(HandleOrRid movieId)
			{
				FrameworkManagers *res = FrameworkManagers::Get();

				if(movieId.isHandle && movieId.handle != HANDLE_NONE) {
					FlashMovieManager::AddRef(movieId.handle);
					movie = movieId.handle;
				} else if(movieId.rid != RID_NONE) {
					movie = res->flashMovieManager->Load(movieId.rid);
				} else {
					Console::Error("Invalid flash movie handle or rid");
					return false;
				}

				if(!state.Init(movie)) {
					Console::Error("Flash movie state init failed");
					return false;
				}
				return true;
			}

			Flash *Flash::Clone(bool prototype)
			{
				Flash *c = ComponentPool<Flash>::Get()->Create();
				c->Init(movie);
				return c;
			}

		} // namespace Components

	} // namespace Framework

} // namespace Maki