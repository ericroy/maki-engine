#pragma once
#include <cstdlib>
#include <iostream>
#include "core/MakiTypes.h"

namespace maki {
	namespace core {

		class document_t {
			MAKI_NO_COPY(document_t);
			friend class document_text_serializer_t;
			friend class document_binary_serializer_t;

		public:
			class node_t {
				MAKI_NO_COPY(node_t);
				friend class document_t;
				friend class document_text_serializer_t;
				friend class document_binary_serializer_t;
		
			public:
				node_t(const char *data, uint32_t length);
				node_t(char *data, uint32_t length, bool should_clone_data);
				~node_t();

				inline uint64_t length() const { return length_; }
				inline const node_t &operator[](uint64_t i) const { return *children_[i]; }
				inline node_t &operator[](uint64_t i) { return *children_[i]; }
				inline node_t *begin() { return children_[0]; }
				inline node_t *end() { return children_[length_]; }
				inline const node_t *begin() const { return children_[0]; }
				inline const node_t *end() const { return children_[length_]; }

				void append_child(node_t *n);
				inline void append_sibling(node_t *n) { MAKI_ASSERT(parent_ != nullptr); parent_->append_child(n); }
				node_t *remove_child(uint32_t index);
				bool remove_child(node_t *n);
				void detach();

				inline const char *value() const { return value_; }

				inline bool value_equals(const char *v) const { return strcmp(v, value_) == 0; }
			
				int64_t value_as_int(int64_t default_value = 0) const;
				bool value_as_int(int64_t *out) const;

				uint64_t value_as_uint(uint64_t default_value = 0) const;
				bool value_as_uint(uint64_t *out) const;

				float value_as_float(float default_value = 0.0f) const;
				bool value_as_float(float *out) const;

				bool value_as_bool(bool default_value = false) const;
				bool value_as_bool(bool *out) const;
			
				// Path resolution helpers:
				const node_t *resolve(const char *node_path) const;
				const char *resolve_value(const char *node_path) const;
				bool resolve_as_int(const char *node_path, int64_t *out) const;
				int64_t resolve_as_int(const char *node_path, int64_t default_value = 0ll) const;
				bool resolve_as_uint(const char *node_path, uint64_t *out) const;
				uint64_t resolve_as_uint(const char *node_path, uint64_t default_value = 0ull) const;
				bool resolve_as_float(const char *node_path, float *out) const;
				float resolve_as_float(const char *node_path, float default_value = 0.0f) const;
				bool resolve_as_bool(const char *node_path, bool *out) const;
				bool resolve_as_bool(const char *node_path, bool default_value = false) const;
				bool resolve_as_vector_n(const char *node_path, uint64_t count, float *out) const;

			private:
				node_t *parent_ = nullptr;
				char *value_ = nullptr;
				uint64_t length_ = 0;
				uint64_t capacity_ = 0;
				node_t **children_ = nullptr;
			};
	
	
		public:
			document_t();
			virtual ~document_t();
		
			inline const node_t &root() const { return *root_; }
			void print(const char *indent_token = "  ");
			bool load(char *data, uint64_t length);
	#ifndef MAKI_TOOLS
			bool load(rid_t rid);
	#endif

		private:
			node_t *root_ = nullptr;
		};


		class document_text_serializer_t {
			MAKI_NO_COPY(document_text_serializer_t);
		private:
			static const int32_t stack_children_count_threshold_ = 4;

		public:
			document_text_serializer_t(document_t &doc);
			bool serialize(char *path, const char *indent_token = "\t");
			bool serialize(::std::ostream &out, const char *indent_token = "\t");
			bool deserialize(char *data, uint32_t length);

		private:
			bool add_node(document_t::node_t *n, int32_t indent_level, document_t::node_t **previous, int32_t &previous_indent_level, bool &clean_line, bool &append_sibling);
			void serialize_node(document_t::node_t *n, uint32_t depth, ::std::ostream &out, bool stacking, bool first_in_stack, bool last_in_stack, const char *indent_token);

		private:
			document_t &doc_;
		};


		class document_binary_serializer_t {
			MAKI_NO_COPY(document_binary_serializer_t);
		private:
			static const uint8_t BINARY_HEADER[4];

		public:
			static bool is_binary_document(char *data, uint32_t length);

		public:
			document_binary_serializer_t(document_t &doc);
			bool serialize(char *path);
			bool serialize(::std::ostream &out);
			bool deserialize(char *data, uint32_t length);

		private:
			uint16_t get_or_add(::std::vector<::std::string> &string_table, char *str);
			void serialize_node(document_t::node_t *n, ::std::ostream &body, ::std::vector<::std::string> &string_table, uint32_t &level);

		private:
			document_t &doc_;
		};


	} // namespace core
} // namespace maki
