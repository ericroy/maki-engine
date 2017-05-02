#pragma once
#include "core/MakiTypes.h"
#include "core/MakiAllocator.h"
#include "core/MakiArray.h"
#include "core/MakiMatrix44.h"
#include "core/MakiDrawCommand.h"


namespace maki {
	namespace core {
	
		class draw_command_list_t {
		public:
			struct __declspec(align(16)) key_entry_t : public aligned_t<16> {
				uint64_t key;
				uint64_t index;
			};

			struct value_entry_t {
				matrix44_t m;
				draw_command_t draw_command;
			};

			static inline bool draw_command_sort_comparator_t(const key_entry_t &a, const key_entry_t &b) {
				return a.key < b.key;
			}

		public:
			draw_command_list_t(uint64_t capacity) {
				keys_.set_length(capacity);
				values_.set_length(capacity);
				values_.zero();
			}
		
			~draw_command_list_t() {}

			inline void add(const draw_command_t &draw_command, const matrix44_t &m) {
				assert(length_ < keys_.length());
				values_[length_].draw_command.copy(draw_command);
				values_[length_].m = m;
				keys_[length_].key = draw_command.get_key();
				keys_[length_].index = length_;
				length_++;
			}

			inline void sort() {
				std::sort(std::begin(keys_), std::end(keys_), draw_command_sort_comparator_t);
			}

			inline void clear() {
				for(uint32_t i = 0; i < length_; i++)
					values_[i].draw_command.clear();
				length_ = 0;
			}

		private:
			array_t<key_entry_t> keys_;
			array_t<value_entry_t> values_;
			uint64_t length_ = 0;
		};

	} // namespace core
} // namespace maki
