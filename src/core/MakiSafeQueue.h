#pragma once
#include "core/core_stdafx.h"
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include "core/MakiArray.h"

namespace maki
{
	namespace core
	{

		template<class T>
		class safe_queue_t
		{
		public:
			static const int32_t default_size_ = 64;

		public:
			safe_queue_t(int32_t size = default_size_)
				: available_(0),
				read_cursor_(0),
				write_cursor_(0)
			{
				items_.set_size(size);
				items_.zero();
			}

			~safe_queue_t()
			{
			}

			inline void put(const T &val)
			{
				std::unique_lock<std::mutex> lock(mutex_);
				while(available_ == items_.count_) {
					cond_space_available_.wait(lock);
				}

				items_[write_cursor_++] = val;
				available_++;
				if(write_cursor_ == items_.count_) {
					write_cursor_ = 0;
				}
				cond_data_available_.notify_one();
			}

			inline bool try_put(const T &val)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if(available_ < items_.count_) {
					items_[write_cursor_++] = val;
					available_++;
					if(write_cursor_ == items_.count_) {
						write_cursor_ = 0;
					}
					cond_data_available_.notify_one();
					return true;
				}
				return false;
			}

			inline void get(T &out)
			{
				std::unique_lock<std::mutex> lock(mutex_);
				while(available_ == 0) {
					cond_data_available_.wait(lock);
				}

				out = items_[read_cursor_++];
				available_--;
				if(read_cursor_ == items_.count_) {
					read_cursor_ = 0;
				}
				cond_space_available_.notify_one();
			}

			inline bool try_get(T &out)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				if(available_ > 0) {
					out = items_[read_cursor_++];
					available_--;
					if(read_cursor_ == items_.count_) {
						read_cursor_ = 0;
					}
					cond_space_available_.notify_one();
					return true;
				}
				return false;
			}

		private:
			uint32_t available_;
		
			std::mutex mutex_;
			std::condition_variable cond_data_available_;
			std::condition_variable cond_space_available_;

			uint32_t read_cursor_;
			uint32_t write_cursor_;
			array_t<T> items_;
		};

	} // namespace core

} // namespace maki
