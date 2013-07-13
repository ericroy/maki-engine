#pragma once
#include "core/core_stdafx.h"
#include "core/MakiDrawCommand.h"


namespace Maki
{
	namespace Core
	{
	
		class DrawCommandList
		{
		public:
			__declspec(align(16)) struct KeyEntry : public Aligned<16>
			{
				uint64 key;
				uint32 index;
			};

			struct ValueEntry
			{
				Matrix44 m;
				DrawCommand drawCommand;
			};

			static inline bool DrawCommandSortComparator(const KeyEntry &a, const KeyEntry &b) { return a.key < b.key; }

		public:
			DrawCommandList(uint32 size)
				: count(0)
			{
				keys.SetSize(size);
				values.SetSize(size);
				values.Zero();
			}
		
			~DrawCommandList() {}

			inline void Add(const DrawCommand &drawCommand, const Matrix44 &m)
			{
				assert(count < keys.count);

				values[count].drawCommand.Copy(drawCommand);
				values[count].m = m;
				keys[count].key = drawCommand.GetKey();
				keys[count].index = count;
				count++;
			}

			inline void Sort()
			{
				std::sort(&keys[0], &keys[count], DrawCommandSortComparator);
			}

			inline void Clear()
			{
				for(uint32 i = 0; i < count; i++) {
					values[i].drawCommand.Clear();
				}
				count = 0;
			}

		public:

			Array<KeyEntry> keys;
			Array<ValueEntry> values;

			uint32 count;
		};

		
		


	} // namespace Core

} // namespace Maki