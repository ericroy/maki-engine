#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiMessage.h"

namespace Maki
{
	namespace Framework
	{
	
		class Entity;

		class SystemBase
		{
		public:
			static uint32 GetSystemCount();
			static void ComponentMakeupChanged(Entity *e, uint64 oldFlags, uint64 newFlags);
			static void DispatchMessages();

		private:
			static std::vector<SystemBase *> systems;

		public:
			SystemBase(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName);
			virtual ~SystemBase();

			inline bool CompatibleWith(uint64 componentFlags) const
			{
				return (requiredComponentMask & componentFlags) == requiredComponentMask &&
					(anyOfComponentMask == 0 || (anyOfComponentMask & componentFlags) != 0);
			}

			inline void PostMessage(const Message &m)
			{
				if(outgoingMessageCount < outgoingMessages.count) {
					outgoingMessages[outgoingMessageCount++] = m;
				} else {
					Console::Error("Outgoing message queue full!  Message being dropped. (%s)", typeName.c_str());
				}
			}

			virtual void Add(Entity *e) = 0;
			virtual void Remove(Entity *e) = 0;
			virtual void ProcessMessages() {}

		private:
			uint64 requiredComponentMask;
			uint64 anyOfComponentMask;
			uint32 outgoingMessageCount;
			Array<Message> outgoingMessages;
			std::string typeName;
		};


		template<typename T1>
		class System1 : public SystemBase
		{
			struct Node
			{
				T1 *t1;

				inline bool operator==(const Node &other) const { return t1 == other.t1; }
				template<typename U> inline U *Get() { return nullptr; }
				template<> inline T1 *Get<T1>() { return t1; }
			};

		public:
			System1(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName)
				: SystemBase(requiredComponentMask, anyOfComponentMask, messageQueueSize, typeName)
			{}
			virtual ~System1() {}
			
		protected:
			void Add(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				nodes.push_back(n);
			}

			void Remove(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		protected:
			std::vector<Node> nodes;
		};


		template<typename T1, typename T2>
		class System2 : public SystemBase
		{
			struct Node
			{
				T1 *t1;
				T2 *t2;

				inline bool operator==(const Node &other) const { return t1 == other.t1; }
				
				template<typename U> inline U *Get() { return nullptr; }
				template<> inline T1 *Get<T1>() { return t1; }
				template<> inline T2 *Get<T2>() { return t2; }
			};

		public:
			System2(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName)
				: SystemBase(requiredComponentMask, anyOfComponentMask, messageQueueSize, typeName)
			{}
			virtual ~System2() {}
			
		protected:
			void Add(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				n.t2 = e->Get<T2>();
				nodes.push_back(n);
			}

			void Remove(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		protected:
			std::vector<Node> nodes;
		};



		template<typename T1, typename T2, typename T3>
		class System3 : public SystemBase
		{
			struct Node
			{
				T1 *t1;
				T2 *t2;
				T3 *t3;

				inline bool operator==(const Node &other) const { return t1 == other.t1; }
				
				template<typename U> inline U *Get() { return nullptr; }
				template<> inline T1 *Get<T1>() { return t1; }
				template<> inline T2 *Get<T2>() { return t2; }
				template<> inline T3 *Get<T3>() { return t3; }
			};

		public:
			System3(uint64 requiredComponentMask, uint64 anyOfComponentMask, uint32 messageQueueSize, const char *typeName)
				: SystemBase(requiredComponentMask, anyOfComponentMask, messageQueueSize, typeName)
			{}
			virtual ~System3() {}
			
		protected:
			void Add(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				n.t2 = e->Get<T2>();
				n.t3 = e->Get<T3>();
				nodes.push_back(n);
			}

			void Remove(Entity *e)
			{
				Node n;
				n.t1 = e->Get<T1>();
				nodes.erase(std::find(std::begin(nodes), std::end(nodes), n));
			}

		protected:
			std::vector<Node> nodes;
		};


	} // namespace Framework

} // namespace Maki