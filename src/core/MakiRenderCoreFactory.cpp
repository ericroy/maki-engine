#include "core/core_stdafx.h"
#include "core/MakiRenderCoreFactory.h"
#include "core/MakiUtil.h"

namespace Maki
{
	namespace Core
	{

		void *RenderCoreFactory::d3dRenderCoreLib = nullptr;
		void *RenderCoreFactory::oglRenderCoreLib = nullptr;
				
		RenderCoreFactory::RenderCoreFactory()
		{
		}

		RenderCoreFactory::~RenderCoreFactory()
		{
		}

		RenderCore *RenderCoreFactory::Create(RenderCore::Type type, Window *window, const Config *config)
		{
			void **libPtr = type == RenderCore::Type_D3D ? &d3dRenderCoreLib : &oglRenderCoreLib;
			void **otherLibPtr = type == RenderCore::Type_D3D ? &oglRenderCoreLib : &d3dRenderCoreLib;

			if(*otherLibPtr != nullptr) {
				// The other library is already loaded, unload it now
				MakiFreeLibrary(*otherLibPtr);
				*otherLibPtr = nullptr;
			}

			if(*libPtr == nullptr) {
				if(type == RenderCore::Type_D3D) {
					*libPtr = MakiLoadLibrary("MakiD3DRenderCore.dll");
					if(*libPtr == nullptr) {
						Console::Error("Failed to load library: MakiD3DRenderCore");
						return nullptr;
					}
					Console::Info("MakiD3DRenderCore loaded");
				} else {
					*libPtr = MakiLoadLibrary("MakiOGLRenderCore.dll");
					if(*libPtr == nullptr) {
						Console::Error("Failed to load library: MakiOGLRenderCore");
						return nullptr;
					}
					Console::Info("MakiOGLRenderCore loaded");
				}
			}
	
			MakiRenderCoreCreateFn create = (MakiRenderCoreCreateFn)MakiGetProcAddress(*libPtr, "CreateRenderCore");
			if(create == nullptr) {
				Console::Error("Failed to get proc address for CreateRenderCore");
				return nullptr;
			}

			RenderCore *core = create(window, config);
			if(core == nullptr) {
				Console::Error("CreateRenderCore failed");
				return nullptr;
			}
			return core;
		}

		void RenderCoreFactory::Destroy(RenderCore *core)
		{
			void **libPtr = d3dRenderCoreLib != nullptr ? &d3dRenderCoreLib : &oglRenderCoreLib;
			assert(*libPtr != nullptr && "No render core lib loaded");

			MakiRenderCoreDestroyFn destroy = (MakiRenderCoreDestroyFn)MakiGetProcAddress(*libPtr, "DestroyRenderCore");
			if(destroy == nullptr) {
				Console::Error("Failed to get proc address for DestroyRenderCore");
				return;
			}

			if(core != nullptr) {
				destroy(core);
			}

			MakiFreeLibrary(*libPtr);
			*libPtr = nullptr;
		}
		
	} // namespace Core

} // namespace Maki