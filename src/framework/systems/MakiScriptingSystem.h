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

			class ScriptingSystem : public SystemBase
			{
				friend class Framework::ScriptingApi;

			public:
				struct  Node
				{
					ScriptingSystem *scriptSys;
					Components::Script *scriptComp;
					inline bool operator==(const Node &other) const { return scriptComp == other.scriptComp; }
				};

			public:
				ScriptingSystem(uint32 messageQueueSize);
				virtual ~ScriptingSystem();
		
				void Update(float dt);
				void ProcessMessages();

			protected:
				void Add(Entity *e);
				void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki