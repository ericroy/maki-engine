#pragma once
#include "core/core_stdafx.h"
#include "core/MakiResource.h"

namespace maki
{
	namespace core
	{

		class vertex_format_t
		{
		public:
			enum Attribute
			{
				Attribute_Position = 0,
				Attribute_Normal,
				Attribute_Tangent,
				Attribute_Color,
				Attribute_Color1,
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
				AttributeFlag_Color1 = 1<<3,
				AttributeFlag_TexCoord = 1<<4,
				AttributeFlag_BoneWeight = 1<<5
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
			vertex_format_t();
			vertex_format_t(const vertex_format_t &other);
			vertex_format_t(const move_token_t<vertex_format_t> &) { assert(false && "vertex_format_t move construction not allowed"); }
			~vertex_format_t();
			inline bool operator==(const vertex_format_t &other) const;
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





		inline bool vertex_format_t::operator==(const vertex_format_t &other) const
		{
			if(attrFlags != other.attrFlags) {
				return false;
			}
			return memcmp(formats, other.formats, sizeof(formats)) == 0;
		}

		bool vertex_format_t::HasAttribute(Attribute attr) const
		{
			return (attrFlags & (1<<attr)) != 0;
		}

		vertex_format_t::DataType vertex_format_t::GetDataType(Attribute attr) const
		{
			return (DataType)formats[attr].type_;
		}

		uint8 vertex_format_t::GetDataCount(Attribute attr) const
		{
			return formats[attr].count_;
		}

		int32 vertex_format_t::GetStride() const
		{
			return stride;
		}

		int32 vertex_format_t::GetAttributeCount() const
		{
			return attrCount;
		}

	} // namespace core

} // namespace maki
