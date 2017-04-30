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
				uint64_t key_;
				uint32_t index_;
			};

			struct value_entry_t
			{
				matrix44_t m_;
				draw_command_t draw_command_;
			};

			static inline bool draw_command_sort_comparator_t(const key_entry_t &a, const key_entry_t &b) { return a.key_ < b.key_; }

		public:
			draw_command_list_t(uint32_t size)
				: count_(0)
			{
				keys_.set_size(size);
				values_.set_size(size);
				values_.zero();
			}
		
			~draw_command_list_t() {}

			inline void add(const draw_command_t &draw_command, const matrix44_t &m)
			{
				assert(count_ < keys_.count_);

				values_[count_].draw_command_.copy(draw_command);
				values_[count_].m_ = m;
				keys_[count_].key_ = draw_command.get_key();
				keys_[count_].index_ = count_;
				count_++;
			}

			inline void sort()
			{
				std::sort(&keys_[0], &keys_[count_], draw_command_sort_comparator_t);
			}

			inline void clear()
			{
				for(uint32_t i = 0; i < count_; i++) {
					values_[i].draw_command_.clear();
				}
				count_ = 0;
			}

		public:

			array_t<key_entry_t> keys_;
			array_t<value_entry_t> values_;

			uint32_t count_;
		};

		
		


	} // namespace core

} // namespace maki
