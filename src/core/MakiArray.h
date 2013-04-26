#pragma once
#include "core/core_stdafx.h"

namespace Maki
{

	template<typename T>
	class Array
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

		Array(Array &&other)
			: count(other.count), data(other.data)
		{
			other.count = 0;
			other.data = nullptr;
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

} // namespace Maki