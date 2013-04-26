#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{

	class VertexFormat
	{
	public:
		enum Attribute
		{
			Attribute_Position = 0,
			Attribute_Normal,
			Attribute_Tangent,
			Attribute_Color,
			Attribute_TexCoord,
			Attribute_BoneWeight,
			AttributeCount
		};

		// These must match the flags in the python script that compiles the meshes
		enum AttributeFlag
		{
			AttributeFlag_Normal = 1<<0,
			AttributeFlag_Tangent = 1<<1,
			AttributeFlag_Color = 1<<2,
			AttributeFlag_TexCoord = 1<<3,
			AttributeFlag_BoneWeight = 1<<4
		};
		
		enum DataType
		{
			DataType_Float = 0,
			DataType_UnsignedInt32,
			DataType_UnsignedInt16,
			DataType_UnsignedInt8,
			DataTypeCount
		};

		static const uint8 DataTypeSizes[DataTypeCount];

	private:
		struct Format
		{
			uint8 type : 2;			// Holds one of the four possible data types from above
			uint8 count : 6;		// Up to 64
		};

		static const uint8 DATA_COUNT_BIT_SHIFT = 3;
		static const uint8 DATA_TYPE_MASK = (1<<DATA_COUNT_BIT_SHIFT) - 1;
		static const uint8 DATA_COUNT_MASK = 0xff-DATA_TYPE_MASK;

	public:
		VertexFormat();
		VertexFormat(VertexFormat &&other) { assert("Move constructor not implemented"); }
		~VertexFormat();

		inline bool operator==(const VertexFormat &other) const
		{
			if(attrFlags != other.attrFlags) {
				return false;
			}
			return memcmp(formats, other.formats, sizeof(formats)) == 0;
		}

		inline bool HasAttribute(Attribute attr) const { return (attrFlags & (1<<attr)) != 0; }
		inline DataType GetDataType(Attribute attr) const { return (DataType)formats[attr].type; }
		inline uint8 GetDataCount(Attribute attr) const { return formats[attr].count; }
		inline int32 GetStride() const { return stride; }
		inline int32 GetAttributeCount() const { return attrCount; }

		void PushAttribute(Attribute attr, DataType type, uint8 count)
		{
			assert(count <= 64);
			attrFlags |= 1<<attr;
			formats[attr].type = type;
			formats[attr].count = count;
			stride += DataTypeSizes[type]*count;
			attrCount++;
		}

	public:
		uint16 stride;
		uint8 attrCount;
		union
		{
			struct
			{
				uint8 attrFlags;
				Format formats[AttributeCount];
			};
			uint64 equalityKey;
		};
	};

} // namespace Maki