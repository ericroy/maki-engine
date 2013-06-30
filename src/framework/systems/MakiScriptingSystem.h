#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		class ScriptingApi;

		namespace Components
		{
			class Script;
		}

		namespace Systems
		{

			class ScriptingSystem : public System
			{
				friend class Framework::ScriptingApi;

			public:
				struct Node
				{
					ScriptingSystem *scriptSys;
					Components::Script *scriptComp;
					inline bool operator==(const Node &other) const { return scriptComp == other.scriptComp; }
				};

			public:
				ScriptingSystem();
				virtual ~ScriptingSystem();
		
				void Update(float dt);
				void ProcessMessages(const std::vector<Message> &messages);

			protected:
				void Add(Entity *e);
				virtual void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
				const std::vector<Message> *currentlyProcessingQueue;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki