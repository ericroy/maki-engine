#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{

		template<typename T>
		class array_t
		{
		public:
			array_t()
				: data_(nullptr),
				count_(0)
			{
			}
		
			array_t(uint32 size)
				: data_(nullptr),
				count_(0)
			{
				set_size(size);
			}

			array_t(const move_token_t<array_t> &other)
				: count_(other.obj->count_), data_(other.obj->data_)
			{
				other.obj->count_ = 0;
				other.obj->data_ = nullptr;
			}

			~array_t()
			{
				SAFE_FREE(data_);
			}

			inline void set_size(uint32 size)
			{
				if(size == count_) {
					return;
				}
				SAFE_FREE(data_);
				data_ = (T *)allocator_t::malloc(sizeof(T)*size, std::alignment_of<T>::value);
				count_ = size;
			}

			inline void zero()
			{
				memset(data_, 0, sizeof(T)*count_);
			}

			inline void free()
			{
				SAFE_FREE(data_);
				count_ = 0;
			}

			inline T &operator[](uint32 index)
			{
				return data_[index];
			}

			inline const T &operator[](uint32 index) const
			{
				return data_[index];
			}

		public:
			T *data_;
			uint32 count_;
		};


	} // namespace core

} // namespace maki
