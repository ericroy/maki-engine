#pragma once
#include "framework/framework_stdafx.h"
#include "core/MakiResourcePool.h"
#include "framework/MakiEntity.h"

namespace Maki
{
	namespace Framework
	{
		// Pool of entities. Manages entity uids.  Each uid has a generation index built into it
		// so if a uid gets recycled, it won't be mistaken for the entity that previously had that uid.
		// No entity will ever have uid==0, it's reserved to mean <no entity>
		class EntityPool : public PseudoSingleton<EntityPool>
		{
		public:
			EntityPool(uint32 size)
				: PseudoSingleton<EntityPool>(),
				resPool(size, "EntityPool"),
				generationCount(nullptr)
			{
				generationCount = (uint32 *)Allocator::Malloc(size * sizeof(uint32));
				for(uint32 i = 0; i < size; i++) {
					// 1 will be the minimum generation number to ensure we never have a uid of 0
					generationCount[i] = 1;
				}
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
				uid |= (uint64)generationCount[index] << 32;

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
				if(generationCount[index] == 0) {
					// 1 will be the minimum generation number to ensure we never have a uid of 0
					generationCount[index]++;
				}
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