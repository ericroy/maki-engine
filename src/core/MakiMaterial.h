#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"
#include "core/MakiShaderProgram.h"

namespace Maki
{

	class Material : public Resource
	{
	public:
		static const int32 MAX_UNIFORMS = 16;

		struct UniformValue
		{
			UniformValue()
				: data(nullptr), bytes(0), vsLocation(-1), psLocation(-1)
			{
			}
			~UniformValue()
			{
				SAFE_FREE(data);
			}

			// Offset within the shader
			int32 vsLocation;
			int32 psLocation;
			
			// Actual data for this constant
			uint32 bytes;
			char *data;
		};

	public:
		Material();
		Material(const MoveToken<Material> &other);
		explicit Material(const Material &other);
		virtual ~Material();
		bool Load(Rid rid);
		void SetShaderProgram(Rid shaderRid);
		void SetTextures(uint8 count, Rid *textureRids);

		// Push a constant buffer and give ownership of the pointer to this object.
		// Pointer must be allocated with Maki::Allocator.
		// Returns the index of the constant value in this material.
		// Returns -1 on failure (provided pointer will be freed in this case).
		int32 PushConstant(const char *key, uint32 bytes, char *data);
		
	public:
		Handle textureSet;
		Handle shaderProgram;
		uint8 uniformCount;
		UniformValue uniformValues[MAX_UNIFORMS];
	};



} // namespace Maki