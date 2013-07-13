#pragma once
#include "core/core_stdafx.h"
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include "core/MakiArray.h"

namespace Maki
{
	namespace Core
	{

		
		

		template<class T>
		class SafeQueue
		{
		public:
			static const int32 DEFAULT_SIZE = 64;

		public:
			SafeQueue(int32 size = DEFAULT_SIZE)
				: available(0),
				readCursor(0),
				writeCursor(0)
			{
				items.SetSize(size);
				items.Zero();
			}

			~SafeQueue()
			{
			}

			inline void Put(const T &val)
			{
				std::unique_lock<std::mutex> lock(mutex);
				while(available == items.count) {
					condSpaceAvailable.wait(lock);
				}

				items[writeCursor++] = val;
				available++;
				if(writeCursor == items.count) {
					writeCursor = 0;
				}
				condDataAvailable.notify_one();
			}

			inline bool TryPut(const T &val)
			{
				std::lock_guard<std::mutex> lock(mutex);
				if(available < items.count) {
					items[writeCursor++] = val;
					available++;
					if(writeCursor == items.count) {
						writeCursor = 0;
					}
					condDataAvailable.notify_one();
					return true;
				}
				return false;
			}

			inline void Get(T &out)
			{
				std::unique_lock<std::mutex> lock(mutex);
				while(available == 0) {
					condDataAvailable.wait(lock);
				}

				out = items[readCursor++];
				available--;
				if(readCursor == items.count) {
					readCursor = 0;
				}
				condSpaceAvailable.notify_one();
			}

			inline bool TryGet(T &out)
			{
				std::lock_guard<std::mutex> lock(mutex);
				if(available > 0) {
					out = items[readCursor++];
					available--;
					if(readCursor == items.count) {
						readCursor = 0;
					}
					condSpaceAvailable.notify_one();
					return true;
				}
				return false;
			}

		private:
			uint32 available;
		
			std::mutex mutex;
			std::condition_variable condDataAvailable;
			std::condition_variable condSpaceAvailable;

			uint32 readCursor;
			uint32 writeCursor;
			Array<T> items;
		};

	} // namespace Core

} // namespace Maki