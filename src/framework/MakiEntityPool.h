#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiResourcePool.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	namespace Framework
	{

		class EntityPool : public PseudoSingleton<EntityPool>
		{
		public:
			EntityPool(uint32 size)
				: PseudoSingleton<EntityPool>(),
				resPool(size, "EntityPool"),
				generationCount(nullptr)
			{
				assert(size < (1ULL<<32));
				generationCount = (uint32 *)Allocator::Malloc(size * sizeof(uint32));
				memset(generationCount, 0, sizeof(uint32) * size);
			}
			virtual ~EntityPool()
			{
				SAFE_FREE(generationCount);
			}
			
			Entity *Create(bool prototype) {
				Handle h = resPool.Alloc();
				Entity *e = resPool.Get(h);
				uint32 index = e - resPool.GetBaseAddr();

				// Create uid for this entity
				uint64 uid = (uint32)h;
				uid |= (uint64)index << 32;

				new(e) Entity(uid, prototype);
				return e;
			}
			
			void Destroy(Entity *e)
			{
				const uint64 &uid = e->GetUid();
				Handle h = (uint32)(uid & 0x00000000ffffffff);
				uint32 index = e - resPool.GetBaseAddr();
				resPool.Free(h);
				generationCount[index]++;
			}
			
			inline Entity *GetEntity(uint64 uid) const
			{
				return resPool.Get((uint32)(uid & 0x00000000ffffffff));
			}

		private:
			ResourcePool<Entity> resPool;
			uint32 *generationCount;
		};

	} // namespace Core

} // namespace Maki