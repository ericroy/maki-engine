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
			enum attribute_t
			{
				attribute_position_ = 0,
				attribute_normal_,
				attribute_tangent_,
				attribute_color_,
				attribute_color1_,
				attribute_tex_coord_,
				attribute_bone_weight_,
				attribute_count_
			};

			// These must match the flags in the python script that compiles the meshes
			enum attribute_flag_t
			{
				attribute_flag_normal_ = 1<<0,
				attribute_flag_tangent_ = 1<<1,
				attribute_flag_color_ = 1<<2,
				attribute_flag_color1_ = 1<<3,
				attribute_flag_text_coord_ = 1<<4,
				attribute_flag_bone_weight_ = 1<<5
			};
		
			enum data_type_t
			{
				data_type_float_ = 0,
				data_type_unsigned_int32_,
				data_type_unsigned_int16_,
				data_type_unsigned_int8_,
				data_type_count_
			};

			static const uint8 data_type_sizes_[data_type_count_];

		private:
			struct format_t
			{
				uint8 type_ : 2;			// Holds one of the four possible data types from above
				uint8 count_ : 6;		// Up to 64
			};

			static const uint8 data_count_bit_shift_ = 3;
			static const uint8 data_type_mask_ = (1<<data_count_bit_shift_) - 1;
			static const uint8 data_count_mask_ = 0xff-data_type_mask_;

		public:
			vertex_format_t();
			vertex_format_t(const vertex_format_t &other);
			vertex_format_t(const move_token_t<vertex_format_t> &) { assert(false && "vertex_format_t move construction not allowed"); }
			~vertex_format_t();
			inline bool operator==(const vertex_format_t &other) const;
			inline bool has_attribute(attribute_t attr) const;
			inline data_type_t get_data_type(attribute_t attr) const;
			inline uint8 get_data_count(attribute_t attr) const;
			inline int32 get_stride() const;
			inline int32 get_attribute_count() const;
			void push_attribute(attribute_t attr, data_type_t type, uint8 count);
			
		public:
			uint16 stride_;
			uint8 attr_count_;
			union
			{
				struct
				{
					uint8 attr_flags_;
					format_t formats_[attribute_count_];
				};
				uint64 equality_key_;
			};
		};





		inline bool vertex_format_t::operator==(const vertex_format_t &other) const
		{
			if(attr_flags_ != other.attr_flags_) {
				return false;
			}
			return memcmp(formats_, other.formats_, sizeof(formats_)) == 0;
		}

		bool vertex_format_t::has_attribute(attribute_t attr) const
		{
			return (attr_flags_ & (1<<attr)) != 0;
		}

		vertex_format_t::data_type_t vertex_format_t::get_data_type(attribute_t attr) const
		{
			return (data_type_t)formats_[attr].type_;
		}

		uint8 vertex_format_t::get_data_count(attribute_t attr) const
		{
			return formats_[attr].count_;
		}

		int32 vertex_format_t::get_stride() const
		{
			return stride_;
		}

		int32 vertex_format_t::get_attribute_count() const
		{
			return attr_count_;
		}

	} // namespace core

} // namespace maki
