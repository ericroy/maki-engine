#pragma once
#include "framework/framework_stdafx.h"
#include "framework/MakiScript.h"

namespace Maki
{
	namespace Framework
	{

		Script::Script()
			: Resource(),
			data(nullptr)
		{
		}

		Script::~Script()
		{
			SAFE_FREE(data);
		}

		bool Script::Load(Rid scriptRid)
		{
			data = Engine::Get()->assets->AllocRead(scriptRid);
			this->rid = scriptRid;
			return true;
		}

	} // namespace Core

} // namespace Maki