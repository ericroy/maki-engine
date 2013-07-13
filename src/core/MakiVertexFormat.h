#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace Maki
{
	namespace Core
	{

		class MAKI_CORE_API VertexFormat
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
			struct MAKI_CORE_API Format
			{
				uint8 type : 2;			// Holds one of the four possible data types from above
				uint8 count : 6;		// Up to 64
			};

			static const uint8 DATA_COUNT_BIT_SHIFT = 3;
			static const uint8 DATA_TYPE_MASK = (1<<DATA_COUNT_BIT_SHIFT) - 1;
			static const uint8 DATA_COUNT_MASK = 0xff-DATA_TYPE_MASK;

		public:
			VertexFormat();
			VertexFormat(const MoveToken<VertexFormat> &) { assert(false && "VertexFormat move construction not allowed"); }
			VertexFormat(const VertexFormat &) { assert(false && "VertexFormat copy construction not allowed"); }
			~VertexFormat();
			inline bool operator==(const VertexFormat &other) const;
			inline bool HasAttribute(Attribute attr) const;
			inline DataType GetDataType(Attribute attr) const;
			inline uint8 GetDataCount(Attribute attr) const;
			inline int32 GetStride() const;
			inline int32 GetAttributeCount() const;
			void PushAttribute(Attribute attr, DataType type, uint8 count);
			
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





		inline bool VertexFormat::operator==(const VertexFormat &other) const
		{
			if(attrFlags != other.attrFlags) {
				return false;
			}
			return memcmp(formats, other.formats, sizeof(formats)) == 0;
		}

		bool VertexFormat::HasAttribute(Attribute attr) const
		{
			return (attrFlags & (1<<attr)) != 0;
		}

		VertexFormat::DataType VertexFormat::GetDataType(Attribute attr) const
		{
			return (DataType)formats[attr].type;
		}

		uint8 VertexFormat::GetDataCount(Attribute attr) const
		{
			return formats[attr].count;
		}

		int32 VertexFormat::GetStride() const
		{
			return stride;
		}

		int32 VertexFormat::GetAttributeCount() const
		{
			return attrCount;
		}

	} // namespace Core

} // namespace Maki