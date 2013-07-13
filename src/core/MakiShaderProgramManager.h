#pragma once
#include "core/core_stdafx.h"
#include "core/MakiManager.h"
#include "core/MakiShaderProgram.h"

namespace Maki
{
	namespace Core
	{

		class ShaderProgramManager;
		MAKI_CORE_API_EXTERN template class MAKI_CORE_API Manager<ShaderProgram, ShaderProgramManager>;

		class MAKI_CORE_API ShaderProgramManager : public Manager<ShaderProgram, ShaderProgramManager>
		{
		public:
			struct MAKI_CORE_API FindPredicate : public std::unary_function<const ShaderProgram *, bool>
			{
				FindPredicate(Rid rid, ShaderProgram::Variant variant) : rid(rid), variant(variant) {}
				inline bool operator()(const ShaderProgram *res) const
				{
					return res->rid == rid && res->variant == variant;
				}
				Rid rid;
				ShaderProgram::Variant variant;
			};

			static const int32 DEFAULT_SIZE = 64;

		public:
			ShaderProgramManager(uint32 size = DEFAULT_SIZE);	
			virtual ~ShaderProgramManager();
			Handle Load(Rid rid, ShaderProgram::Variant variant = ShaderProgram::Variant_Normal);
			void ReloadAssets();
			bool ReloadAsset(Rid rid);
		};

	} // namespace Core

} // namespace Maki
