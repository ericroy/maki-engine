#pragma once
#include "core/core_stdafx.h"
#include "core/MakiDrawCommand.h"


namespace maki
{
	namespace core
	{
	
		class draw_command_list_t
		{
		public:
			struct __declspec(align(16)) key_entry_t : public aligned_t<16>
			{
				uint64 key_;
				uint32 index_;
			};

			struct value_entry_t
			{
				Matrix44 m_;
				draw_command_t draw_command_;
			};

			static inline bool draw_command_sort_comparator_t(const key_entry_t &a, const key_entry_t &b) { return a.key_ < b.key_; }

		public:
			draw_command_list_t(uint32 size)
				: count_(0)
			{
				keys_.set_size(size);
				values_.set_size(size);
				values_.zero();
			}
		
			~draw_command_list_t() {}

			inline void add(const draw_command_t &draw_command, const Matrix44 &m)
			{
				assert(count < keys.count_);

				values[count].draw_command_.copy(draw_command);
				values[count].m_ = m;
				keys[count].key_ = draw_command.get_key();
				keys[count].index_ = count;
				count++;
			}

			inline void sort()
			{
				std::sort(&keys_[0], &keys_[count_], draw_command_sort_comparator_t);
			}

			inline void clear()
			{
				for(uint32 i = 0; i < count_; i++) {
					values_[i].draw_command_.clear();
				}
				count_ = 0;
			}

		public:

			array_t<key_entry_t> keys_;
			array_t<value_entry_t> values_;

			uint32 count_;
		};

		
		


	} // namespace core

} // namespace maki
