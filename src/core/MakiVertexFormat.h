#pragma once
#include "core/MakiTypes.h"
#include "core/MakiResource.h"

namespace maki {
	namespace core {

		class vertex_format_t {
		public:
			enum attribute_t : uint8_t {
				attribute_position = 0,
				attribute_normal,
				attribute_tangent,
				attribute_color,
				attribute_color1,
				attribute_tex_coord,
				attribute_bone_weight,
				attribute_max = attribute_bone_weight
			};

			// These must match the flags in the python script that compiles the meshes
			enum attribute_flag_t : uint8_t {
				attribute_flag_normal = 1 << 0,
				attribute_flag_tangent = 1 << 1,
				attribute_flag_color = 1 << 2,
				attribute_flag_color1 = 1 << 3,
				attribute_flag_text_coord = 1 << 4,
				attribute_flag_bone_weight = 1 << 5,
				attribute_flag_max = attribute_flag_bone_weight
			};
		
			enum data_type_t : uint8_t {
				data_type_float = 0,
				data_type_unsigned_int32,
				data_type_unsigned_int16,
				data_type_unsigned_int8,
				data_type_max = data_type_unsigned_int8
			};

			static const uint8_t data_type_sizes[data_type_max + 1];

		private:
			struct format_t {
				uint8_t type : 2;		// Holds one of the four possible data types from above
				uint8_t count : 6;		// Up to 64
			};

			static const uint8_t data_count_bit_shift_ = 3;
			static const uint8_t data_type_mask_ = (1 << data_count_bit_shift_) - 1;
			static const uint8_t data_count_mask_ = 0xff - data_type_mask_;

		public:
			inline bool operator==(const vertex_format_t &other) const {
				return equality_key_ == other.equality_key_;
			}
			
			inline bool has_attribute(attribute_t attr) const {
				return (attr_flags_ & (1 << attr)) != 0;
			}
			
			inline data_type_t data_type(attribute_t attr) const {
				return (data_type_t)formats_[attr].type;
			}
			
			inline uint8_t data_count(attribute_t attr) const {
				return formats_[attr].count;
			}
			
			inline uint16_t stride() const {
				return stride_;
			}
			
			inline uint8_t attribute_count() const {
				return attr_count_;
			}

			void push_attribute(attribute_t attr, data_type_t type, uint8_t count);
			
		private:
			uint16_t stride_ = 0;
			uint8_t attr_count_ = 0;
			union {
				struct {
					uint8_t attr_flags_;
					format_t formats_[attribute_max + 1];
				};
				uint64_t equality_key_ = 0;
			};
		};


	} // namespace core
} // namespace maki
