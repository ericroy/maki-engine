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
		
			array_t(uint32_t size)
				: data_(nullptr),
				count_(0)
			{
				set_size(size);
			}

			array_t(const move_token_t<array_t> &other)
				: count_(other.obj_->count_), data_(other.obj_->data_)
			{
				other.obj_->count_ = 0;
				other.obj_->data_ = nullptr;
			}

			~array_t()
			{
				MAKI_SAFE_FREE(data_);
			}

			inline void set_size(uint32_t size)
			{
				if(size == count_) {
					return;
				}
				MAKI_SAFE_FREE(data_);
				data_ = (T *)allocator_t::malloc(sizeof(T)*size, std::alignment_of<T>::value);
				count_ = size;
			}

			inline void zero()
			{
				memset(data_, 0, sizeof(T)*count_);
			}

			inline void free()
			{
				MAKI_SAFE_FREE(data_);
				count_ = 0;
			}

			inline T &operator[](uint32_t index)
			{
				return data_[index];
			}

			inline const T &operator[](uint32_t index) const
			{
				return data_[index];
			}

		public:
			T *data_;
			uint32_t count_;
		};


	} // namespace core

} // namespace maki
