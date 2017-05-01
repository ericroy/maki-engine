#pragma once
#include "core/MakiTypes.h"
#include "core/MakiMacros.h"
#include "core/MakiAllocator.h"

namespace maki {
	namespace core {

		template<typename T>
		class array_t
		{
			// Can't use MAKI_NO_COPY here because it's a template type
			array_t(const array_t<T> &) = delete;
			void operator=(const array_t<T> &) = delete;

		public:
			array_t() = default;
			
			array_t(uint64_t length) {
				set_length(length);
			}

			array_t(array_t &&other) : length_(other.length_), data_(other.data_) {
				other.length_ = 0;
				other.data_ = nullptr;
			}

			~array_t() {
				MAKI_SAFE_FREE(data_);
			}

			inline const T *begin() const {
				return data_;
			}

			inline T *begin() {
				return data_;
			}

			inline const T *end() const {
				return data_ + length_;
			}

			inline T *end() {
				return data_ + length_;
			}

			inline uint64_t length() const {
				return length_;
			}

			inline T *data() {
				return data_;
			}

			inline const T *data() const {
				return data_;
			}

			void set_length(uint64_t length) {
				if(length_ == length)
					return;
				MAKI_SAFE_FREE(data_);
				data_ = (T *)allocator_t::malloc(sizeof(T) * length, std::alignment_of<T>::value);
				length_ = length;
			}

			inline void zero() {
				memset(data_, 0, sizeof(T) * length_);
			}

			inline void free() {
				MAKI_SAFE_FREE(data_);
				length_ = 0;
			}

			inline T &operator[](uint64_t index) {
				return data_[index];
			}

			inline const T &operator[](uint64_t index) const {
				return data_[index];
			}

		private:
			T *data_ = nullptr;
			uint64_t length_ = 0;
		};


	} // namespace core
} // namespace maki
