#pragma once
#include "core/MakiTypes.h"
#include "core/MakiAllocator.h"

namespace maki {
	namespace core {

		template<typename T> class array_t {
		public:
			array_t() = default;
			array_t(decltype(nullptr)) {}
			
			array_t(size_t length) {
				set_length(length);
			}

			array_t(array_t<T> &&other) {
				std::swap(length_, other.length_);
				std::swap(data_, other.data_);
			}

			array_t(const array_t<T> &other) {
				if (other.length_ > 0) {
					set_length(other.length_);
					memcpy(data_, other.data_, length_);
				}
			}

			~array_t() {
				MAKI_SAFE_FREE(data_);
			}

			inline void operator=(const array_t<T> &other) {
				if (&other != this) {
					free();
					set_length(other.length_);
					memcpy(data_, other.data_, length_);
				}
			}

			inline void operator=(array_t<T> &&other) {
				if (&other != this) {
					free();
					std::swap(length_, other.length_);
					std::swap(data_, other.data_);
				}
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

			inline size_t length() const {
				return length_;
			}

			inline T *data() {
				return data_;
			}

			inline const T *data() const {
				return data_;
			}

			void set_length(size_t length) {
				if (length_ == length)
					return;
				if (length == 0) {
					free();
					return;
				}
				data_ = (T *)allocator_t::realloc(data_, sizeof(T) * length, std::alignment_of<T>::value);
				length_ = length;
			}

			inline void zero() {
				if (data_ != nullptr)
					memset(data_, 0, sizeof(T) * length_);
			}

			inline void free() {
				MAKI_SAFE_FREE(data_);
				length_ = 0;
			}

			inline T &operator[](size_t index) {
				return data_[index];
			}

			inline const T &operator[](size_t index) const {
				return data_[index];
			}

			inline operator bool() const {
				return data_ != nullptr;
			}

		private:
			T *data_ = nullptr;
			size_t length_ = 0;
		};


	} // namespace core
} // namespace maki
