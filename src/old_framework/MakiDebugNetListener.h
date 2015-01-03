#pragma once
#include "framework/framework_stdafx.h"
#include <map>

#if defined(_WIN32) || defined(_WIN64)

namespace Maki
{
	namespace Framework
	{

		class DebugNetListener
		{
			struct Context;

		private:
			static const int32 PORT = 11001;
			static const int32 MAX_BUFFER_LENGTH = 4096;

		public:
			DebugNetListener();
			virtual ~DebugNetListener();
			void Tick();
			inline void SetListener(std::function<void(Rid)> f) { genericListener = f; }
			inline void SetListener(Rid rid, std::function<void(Rid)> f) { listeners[rid] = f; }

		private:
			void FireNotification(Rid rid);

		private:
			Context *context;
			char recvBuf[MAX_BUFFER_LENGTH+1];
			std::function<void(Rid)> genericListener;
			std::map<Rid, std::function<void(Rid)>> listeners;
		};

	} // namespace Framework

} // namespace Maki

#endif
