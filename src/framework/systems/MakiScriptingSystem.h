#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiSystem.h"
#include "framework/MakiComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{
			class Script;
		}

		namespace Systems
		{

			class ScriptingSystem : public System
			{
			private:
				struct Node
				{
					Components::Script *scriptComp;
					inline bool operator==(const Node &other) const { return scriptComp == other.scriptComp; }
				};

			public:
				ScriptingSystem();
				virtual ~ScriptingSystem();
		
				void Update(float dt);

			protected:
				void Add(Entity *e);
				virtual void Remove(Entity *e);

			private:
				std::vector<Node> nodes;
			};

		} // namespace Systems

	} // namespace Framework

} // namespace Maki