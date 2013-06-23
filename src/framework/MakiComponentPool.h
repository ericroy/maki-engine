#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	class TransformComponent;
	class SkeletonComponent;
	class SceneNodeComponent;
	class MeshComponent;
	class LightComponent;
	class CameraComponent;

	class ComponentPool : public PseudoSingleton<ComponentPool>
	{
	public:
		static const uint32 DEFAULT_POOL_SIZE = 128;

	public:
		ComponentPool(Config *config)
			: PseudoSingleton<ComponentPool>(),
			transformComponents(config->GetUint("transform_component_pool_size", DEFAULT_POOL_SIZE)),
			skeletonComponents(config->GetUint("skeleton_component_pool_size", DEFAULT_POOL_SIZE)),
			sceneNodeComponents(config->GetUint("scene_node_component_pool_size", DEFAULT_POOL_SIZE)),
			meshComponents(config->GetUint("mesh_component_pool_size", DEFAULT_POOL_SIZE)),
			lightComponents(config->GetUint("light_component_pool_size", DEFAULT_POOL_SIZE)),
			cameraComponents(config->GetUint("camera_component_pool_size", DEFAULT_POOL_SIZE))
		{
			poolForType[Component::Type_Transform] = &transformComponents;
			poolForType[Component::Type_Skeleton] = &skeletonComponents;
			poolForType[Component::Type_SceneNode] = &sceneNodeComponents;
			poolForType[Component::Type_Mesh] = &meshComponents;
			poolForType[Component::Type_Light] = &lightComponents;
			poolForType[Component::Type_Camera] = &cameraComponents;
		}
		
		virtual ~ComponentPool()
		{
		}

		template<class T>
		T *Create()
		{
			auto pool = (ResourcePool<T> *)poolForType[T::COMPONENT_TYPE];
			Handle h = pool->Alloc();
			assert(h != HANDLE_NONE);

			// This is a little strange, but we don't want to manage these objects with handles.
			// Instead, we will give out a pointer directly to the data, and leak the handle.
			// When the data wants to be deleted, we'll calculate what the handle was, and then free it properly.
			// I should really just write a memory allocator.
			T *c = pool->Get(h);
			new(c) T();
			return c;
		}

		template<class T>
		void Destroy(T *c)
		{
			auto pool = (ResourcePool<T> *)poolForType[T::COMPONENT_TYPE];
			Handle h = c - pool->GetBaseAddr();
			pool->Free(h);
		}		

	private:
		ResourcePool<TransformComponent> transformComponents;
		ResourcePool<SkeletonComponent> skeletonComponents;
		ResourcePool<SceneNodeComponent> sceneNodeComponents;
		ResourcePool<MeshComponent> meshComponents;
		ResourcePool<LightComponent> lightComponents;
		ResourcePool<CameraComponent> cameraComponents;

		void *poolForType[Component::TypeCount];
	};

} // namespace Maki