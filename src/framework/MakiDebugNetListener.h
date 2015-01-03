#pragma once
#include "framework/framework_stdafx.h"
#include <map>

#if defined(_WIN32) || defined(_WIN64)

namespace maki
{
	namespace framework
	{

		class debug_net_listener_t
		{
			struct context_t;

		private:
			static const int32 port_ = 11001;
			static const int32 max_buffer_length_ = 4096;

		public:
			debug_net_listener_t();
			virtual ~debug_net_listener_t();
			void tick();
			inline void set_listener(std::function<void(rid_t)> f) { generic_listener_ = f; }
			inline void set_listener(rid_t rid, std::function<void(rid_t)> f) { listeners_[rid] = f; }

		private:
			void fire_notification(rid_t rid);

		private:
			context_t *context_;
			char recv_buffer_[max_buffer_length_+1];
			std::function<void(rid_t)> generic_listener_;
			std::map<rid_t, std::function<void(rid_t)>> listeners_;
		};

	} // namespace framework

} // namespace maki

#endif
