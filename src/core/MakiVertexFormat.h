#pragma once
#include "core/MakiResource.h"

namespace maki {
	namespace core {

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

			static const uint8_t data_type_sizes_[data_type_count_];

		private:
			struct format_t
			{
				uint8_t type_ : 2;			// Holds one of the four possible data types from above
				uint8_t count_ : 6;		// Up to 64
			};

			static const uint8_t data_count_bit_shift_ = 3;
			static const uint8_t data_type_mask_ = (1<<data_count_bit_shift_) - 1;
			static const uint8_t data_count_mask_ = 0xff-data_type_mask_;

		public:
			vertex_format_t();
			vertex_format_t(const vertex_format_t &other);
			vertex_format_t(vertex_format_t &&) { assert(false && "vertex_format_t move construction not allowed"); }
			~vertex_format_t();
			inline bool operator==(const vertex_format_t &other) const;
			inline bool has_attribute(attribute_t attr) const;
			inline data_type_t get_data_type(attribute_t attr) const;
			inline uint8_t get_data_count(attribute_t attr) const;
			inline int32_t get_stride() const;
			inline int32_t get_attribute_count() const;
			void push_attribute(attribute_t attr, data_type_t type, uint8_t count);
			
		public:
			uint16_t stride_;
			uint8_t attr_count_;
			union
			{
				struct
				{
					uint8_t attr_flags_;
					format_t formats_[attribute_count_];
				};
				uint64_t equality_key_;
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

		uint8_t vertex_format_t::get_data_count(attribute_t attr) const
		{
			return formats_[attr].count_;
		}

		int32_t vertex_format_t::get_stride() const
		{
			return stride_;
		}

		int32_t vertex_format_t::get_attribute_count() const
		{
			return attr_count_;
		}

	} // namespace core
} // namespace maki
