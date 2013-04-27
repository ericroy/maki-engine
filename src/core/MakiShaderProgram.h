#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiShader.h"

namespace Maki
{

	class ShaderProgram : public Resource
	{
	public:
		enum Variant
		{
			Variant_Normal = 0,
			Variant_Depth,
			Variant_Shadow,
			VariantCount
		};

	private:
		static char *variantDataKey[VariantCount];
		static char *variantMetaKey[VariantCount];

	public:
		ShaderProgram();
		~ShaderProgram();

		bool Load(Rid rid, Variant variant);

		inline bool operator==(const ShaderProgram &other) const
		{
			return rid == other.rid && variant == other.variant;
		}

	private:
		ShaderProgram(const ShaderProgram &) {}

	public:
		Shader vertexShader;
		Shader pixelShader;
		Variant variant;
		Handle variants[VariantCount-1];
	};

} // namespace Maki
