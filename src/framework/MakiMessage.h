#pragma once
#include "framework/framework_stdafx.h"
#include "framework/hash/hash.h"

namespace Maki
{
	namespace Framework
	{

		MAKI_DECLARE_HASH(Message_Ping);
		MAKI_DECLARE_HASH(Message_Pong);
		MAKI_DECLARE_HASH(Message_PlayTrack);
		MAKI_DECLARE_HASH(Message_TrackComplete);


		// Message from one Entity to another.  The <to> field can be omitted (or you can use zero) if you want
		// to broadcast the message to all entities.
		struct  Message
		{
		public:
			template<class T, class U>
			Message(const uint64 &from, const uint64 &to, uint32 msg, T &arg1, U &arg2)
				: from(from), to(to), msg(msg), arg1(arg1), arg2(arg2)
			{
				this->arg1 = static_cast<uintptr_t>(arg1);
				this->arg2 = static_cast<uintptr_t>(arg2);
			}

			template<class T>
			Message(const uint64 &from, const uint64 &to, uint32 msg, T &arg1)
				: from(from), to(to), msg(msg), arg2(0)
			{
				this->arg1 = static_cast<uintptr_t>(arg1);
			}

			Message(const uint64 &from, const uint64 &to, uint32 msg)
				: from(from), to(to), msg(msg), arg1(0), arg2(0)
			{
			}

			Message(const Message &other)
				: from(other.from), to(other.to), msg(other.msg), arg1(other.arg1), arg2(other.arg2)
			{
			}

			template<class T>
			inline T GetArg1() const { return reinterpret_cast<T>(arg1); }

			template<class T>
			inline T GetArg2() const { return reinterpret_cast<T>(arg2); }

		public:
			uint64 from;
			uint64 to;
			uintptr_t arg1;
			uintptr_t arg2;
			uint32 msg;
		};

	} // namespace Framework

} // namespace Maki