#pragma once
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

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
			attribute_flag_tex_coord = 1 << 4,
			attribute_flag_bone_weight = 1 << 5,
			attribute_flag_max = attribute_flag_bone_weight
		};

		enum attribute_type_t : uint8_t {
			attribute_type_float = 0,
			attribute_type_unsigned_int32,
			attribute_type_unsigned_int16,
			attribute_type_unsigned_int8,
			attribute_type_max = attribute_type_unsigned_int8
		};


		class vertex_format_t {
		private:
			struct attribute_desc_t {
				uint8_t type : 2;				// Holds one of the four possible data types from above
				uint8_t element_count : 6;		// Up to 64
			};

		public:
			inline bool operator==(const vertex_format_t &other) const {
				return comparison_key_ == other.comparison_key_;
			}
			
			inline bool has_attribute(attribute_t attr) const {
				return (attribute_flags_ & (1 << attr)) != 0;
			}
			
			inline attribute_type_t get_attribute_type(attribute_t attr) const {
				return (attribute_type_t)attribute_descriptors_[attr].type;
			}
			
			inline uint8_t get_attribute_element_count(attribute_t attr) const {
				return attribute_descriptors_[attr].element_count;
			}
			
			inline uint16_t stride() const {
				return stride_;
			}
			
			inline uint8_t attribute_count() const {
				return attribute_count_;
			}

			void push_attribute(attribute_t attr, attribute_type_t type, uint8_t element_count);
			
		private:
			union {
				struct {
					uint8_t attribute_flags_;
					attribute_desc_t attribute_descriptors_[attribute_max + 1];
				};
				uint64_t comparison_key_ = 0;
			};
			uint16_t stride_ = 0;
			uint8_t attribute_count_ = 0;
		};


	} // namespace core
} // namespace maki
