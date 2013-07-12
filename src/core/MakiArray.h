#pragma once
#include "core/core_stdafx.h"

namespace Maki
{
	namespace Core
	{

		template<typename T>
		class MAKI_CORE_API Array
		{
		public:
			Array()
				: data(nullptr),
				count(0)
			{
			}
		
			Array(uint32 size)
				: data(nullptr),
				count(0)
			{
				SetSize(size);
			}

			Array(const MoveToken<Array> &other)
				: count(other.obj->count), data(other.obj->data)
			{
				other.obj->count = 0;
				other.obj->data = nullptr;
			}

			~Array()
			{
				SAFE_FREE(data);
			}

			inline void SetSize(uint32 size)
			{
				if(size == count) {
					return;
				}
				SAFE_FREE(data);
				data = (T *)Allocator::Malloc(sizeof(T)*size, std::alignment_of<T>::value);
				count = size;
			}

			inline void Zero()
			{
				memset(data, 0, sizeof(T)*count);
			}

			inline void Delete()
			{
				SAFE_FREE(data);
				count = 0;
			}

			inline T &operator[](uint32 index)
			{
				return data[index];
			}

			inline const T &operator[](uint32 index) const
			{
				return data[index];
			}

		public:
			T *data;
			uint32 count;
		};

	} // namespace Core

} // namespace Maki