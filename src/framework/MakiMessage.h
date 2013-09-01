#pragma once
#include "framework/framework_stdafx.h"

namespace Maki
{
	namespace Framework
	{
		// Message from one Entity to another.  The <to> field can be omitted (or you can use zero) if you want
		// to broadcast the message to all entities.
		struct  Message
		{
		public:
			enum
			{
				Message_Ping = 0u,
				Message_Pong,
				MessageCount
			};

		public:
			template<class T, class U>
			Message(const uint64 &from, const uint64 &to, int32 msg, T *arg1, U *arg2)
				: from(from), to(to), msg(msg)
			{
				this->arg1 = reinterpret_cast<uintptr_t>(arg1);
				this->arg2 = reinterpret_cast<uintptr_t>(arg2);
			}

			template<class T>
			Message(const uint64 &from, const uint64 &to, int32 msg, T *arg1)
				: from(from), to(to), msg(msg), arg2(0)
			{
				this->arg1 = reinterpret_cast<uintptr_t>(arg1);
			}

			Message(const uint64 &from, const uint64 &to, int32 msg)
				: from(from), to(to), msg(msg), arg1(0), arg2(0)
			{
			}

			Message(const uint64 &from, int32 msg)
				: from(from), to(0), msg(msg), arg1(0), arg2(0)
			{
			}

			Message(const Message &other)
				: from(other.from), to(other.to), msg(other.msg), arg1(other.arg1), arg2(other.arg2)
			{
			}

			template<class T>
			inline T *GetArg1() const { return reinterpret_cast<T *>(arg1); }

			template<class T>
			inline T *GetArg2() const { return reinterpret_cast<T *>(arg2); }

		public:
			uint64 from;
			uint64 to;
			uintptr_t arg1;
			uintptr_t arg2;
			int32 msg;
		};
		

	} // namespace Framework

} // namespace Maki